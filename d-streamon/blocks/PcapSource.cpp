/**
 * <blockinfo type="PcapSource" scheduling_type="True" thread_exclusive="True">
 *   <humandesc>
 *     Captures traffic from a local interface or pcap trace file and outputs
 *     packets using a Packet message. If the source parameter is set to live,
 *     the name parameter should be an interface's name (e.g., eth0). If the
 *     source type is set to trace, then the name should be the full path to a
 *     pcap trace file (e.g., /tmp/mytrace.pcap). Use the bfp paramter to filter
 *     traffic using BFP syntax.
 *   </humandesc>
 *
 *   <shortdesc>Captures traffic from a local interface or pcap trace file</shortdesc>
 *
 *   <gates>
 *     <gate type="output" name="sniffer_out" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      element source {
 *        attribute type {"live" | "trace"}
 *        attribute name {text}
 *      }
 *      element bpf_filter {text}?
 *    }
 *   </paramsschema>
 *
 *   <paramsexample>
 *     <params>
 *        <source type="live" name="eth0">
 *        or
 *        <source type="trace" name="trace.pcap">
 *
 *      <bpf_filter expression=" ">
 *     </params>
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */
#include <Block.hpp>
#include <pcap.h>
#include <pugixml.hpp>
#include <BlockFactory.hpp>
#include <iostream>
#include <PacketPlus.hpp>
#include <streamon/DnsHeader.hpp>
#include <streamon/DnsResponse.hpp>

using namespace pugi;

namespace bm
{
    /**
     * Implements a block that captures packets from a standard pcap sniffer and injects them into BM as Packet messages
     */

	int g_pkt_count;


    class PcapSource: public Block
    {
        pcap_t* m_source;
        bool m_live;	
        int m_gate_id;

        int pkt_count;
		int filtered_packets;
		int bad_packets;

		uint64_t timestamp_start;
		uint64_t timestamp_curr;

        std::shared_ptr<MemoryBatch> m_mem_block;

        PcapSource(const PcapSource &) = delete;
        PcapSource& operator=(const PcapSource &) = delete;


		void print_stats()
		{
			std::cout <<"\n\nTotal packets: "<< pkt_count <<"\n";
			std::cout << "Filtered packets: " << filtered_packets << "\n";
			std::cout << "Bad packets     : " << bad_packets << "\n";
 
    		std::cout << "Packets at FeatureLayer: " << g_pkt_count << "\n";

			std::cout << "Trace duration (sec.): " << (timestamp_curr - timestamp_start) << "\n\n";
		}

	
    public:

        /**
         * @brief Constructor
         * @param name         The name of the sniffer block
         * @param active       Whether the block should be active or passive
         */	

        PcapSource(const std::string &name, bool active)
        :Block(name, true), //ignore options, sniffer must be active
        m_source(NULL),
        m_live(false), 
        m_gate_id(register_output_gate("sniffer_out")),
		pkt_count(0),
		filtered_packets(0),
		bad_packets(0),
		timestamp_start(0),
		timestamp_curr(0),
        m_mem_block(new MemoryBatch(4096*4))
        {
            if (!active) {
                blocklog("PcapSource must be active, ignoring configuration", log_warning);
            }

			std::cout << "[PcapSource] BUFSIZ: " << BUFSIZ << "\n";
        }

        /**
         * @brief Destructor
         */
        virtual ~PcapSource()
        {
            if(m_source)
                pcap_close(m_source);

			print_stats();
        }


        /**
         * @brief Configures the block: defines the capture interface, whether it's a trace or a live socket and (optional filters)
         * also, it opens the socket for capturing
         * @param n The configuration parameters 
         */
        virtual void _configure(const xml_node& n)
        {
            xml_node source=n.child("source");
            if(!source) 
                throw std::runtime_error("pcapsniffer: missing parameter source");
            std::string type=source.attribute("type").value();
            std::string name=source.attribute("name").value();
            if((type.length()==0)||(name.length()==0))
                throw std::runtime_error("pcapsniffer: missing attribute");
            if(type.compare("live")==0)
                m_live=true;
            else if(type.compare("trace")==0)
                m_live=false;
            else throw std::runtime_error("pcapsniffer: invalid type parameter");
            char errbuf[PCAP_ERRBUF_SIZE];
            if(m_live)
            {
                m_source = pcap_open_live(name.c_str(),BUFSIZ,1 ,10,errbuf);
                if(!m_source)
                { 
                    blocklog(std::string(errbuf), log_error);
                    throw(std::invalid_argument("TcpPcapSource::error within pcap_open_live"));
                }
            }
            else
            {
                m_source=pcap_open_offline(name.c_str(),errbuf);
                if (m_source == NULL) {
                    blocklog(std::string(errbuf), log_error);
                    throw(std::invalid_argument("TcpPcapSource::error within pcap_open_offline"));
                }
            }

            xml_node bpf=n.child("bpf_filter");
            if(bpf)
            {
                bpf_u_int32 mask=0;
                bpf_u_int32 net=0;
                if(m_live)
                {
                    if (pcap_lookupnet(name.c_str(), &net, &mask, errbuf) == -1) {
                        blocklog("Can't get netmask for device", log_error);
                        net = 0;
                        mask = 0;
                    }
                }

                std::string exp=bpf.attribute("expression").value();
                if(exp.length()==0)
                    throw std::runtime_error("pcapsniffer: no bpf expression");
                struct bpf_program fp;
                if (pcap_compile(m_source, &fp, exp.c_str(), 0, net) == -1) {
                    blocklog(pcap_geterr(m_source), log_error);
                    throw(std::invalid_argument("TcpPcapSource::error within pcap_compile"));
                }
                if (pcap_setfilter(m_source, &fp) == -1) {
                    blocklog(pcap_geterr(m_source), log_error);
                    throw(std::invalid_argument("TcpPcapSource::error within pcap_setfilter"));
                }
            }
        }


        
        /** 
          * listens on the capture socket, retrieves packets and sends them out as Packet messages
          * Uses optimized allocation.
          * As the socket call is blocking (although it returns after a timeout expires) this cannot share a thread
          */
        void _do_async_processing()
        {
	    
            struct pcap_pkthdr* hdr=NULL;
            const u_char* pkt = NULL;
            int ret_code = pcap_next_ex(m_source,&hdr,&pkt);

            while (ret_code == 1)
            {
                timespec p_tstamp;
                p_tstamp.tv_sec = hdr->ts.tv_sec;
                p_tstamp.tv_nsec = hdr->ts.tv_usec*1000;

                pkt_count++;

				/* try to parse only DNS

				DnsHeader head = DnsHeader::parse( pkt + 14, hdr->caplen - 14 );

				if (!!head) // if dns header good
				{
					try
					{
						DnsResponse resp = DnsResponse::parse(head);

						if (!resp)
						{
							filtered_packets++;

							// std::cout << "packet #" << pkt_count << ", not valid response.\n";
						}
					}
					catch(std::runtime_error err)
					{
						bad_packets++;
						std::cout << "packet #" << pkt_count << ", exception: " << err.what() << "\n";
					}
				}
				else // not DNS
				{
					filtered_packets++;
				}
				*/

				try
				{
                	std::shared_ptr<const Msg>&& sp(std::move(alloc_msg_from_buffer<PacketPlus>(m_mem_block, (size_t)(hdr->caplen), const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(pkt), hdr->caplen), p_tstamp, hdr->len )));
				
                	auto pktp = static_cast<const PacketPlus*>(sp.get());
				                
                	// ip fragmentation patch

                	if ( !pktp->is_first_fragment() )
                	{
						if (filtered_packets == 0)
						{
							;//std::cout << "Please see the packet #" << pkt_count << ", seems IP fragmented.\n";
						}

						filtered_packets++;

                    	pktp->~PacketPlus();
                	}
					else
					{
						uint64_t timestamp_read = pktp->timestamp_s();

						if (timestamp_start == 0) {
							timestamp_start = timestamp_read;
						}

						if (timestamp_read > timestamp_curr)
							timestamp_curr = timestamp_read;

                		send_out_through(std::move(sp), m_gate_id);
					}
				}
				catch (std::runtime_error err)
				{
					bad_packets++;

					std::cerr << "packet #" << pkt_count << ", exception: " << err.what() << "\n";
				}
		
		
		//Packet* packet = new Packet(m_mem_block, (size_t)(hdr->caplen), const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(pkt), hdr->caplen), p_tstamp, hdr->len );

                ret_code = pcap_next_ex(m_source, &hdr, &pkt);
            }

            if (ret_code == -1)
            {
                blocklog(std::string(pcap_geterr(m_source)), log_warning);
            } else if(ret_code == 0) {
                //timeout expired
		
            } else if(ret_code == -2) {
                //blocklog("trace is over", log_info);
			    // throw(std::invalid_argument("Trace is over"));

				print_stats();

                exit(-1);        

            }
        }
    };

    REGISTER_BLOCK(PcapSource,"PcapSource");
}
