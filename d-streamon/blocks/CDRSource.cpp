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
// #include <pcap.h>
#include <pugixml.hpp>
#include <BlockFactory.hpp>

#include <CDRMessage.hpp>

// #include "CounterIP_S.hpp"

#include <fstream>
//#include "14FeatureLayerZero.cpp"

using namespace pugi;

namespace bm
{
    /**
     * Implements a block that captures packets from a standard pcap sniffer and injects them into BM as Packet messages
     */

    // int g_pkt_count;

    class CDRSource: public Block
    {	
        std::ifstream* m_source;
        int m_gate_id;
	int pkt_count;
        std::shared_ptr<MemoryBatch> m_mem_block;

        CDRSource(const CDRSource &) = delete;
        CDRSource& operator=(const CDRSource &) = delete;

	
    public:

        /**
         * @brief Constructor
         * @param name         The name of the sniffer block
         * @param active       Whether the block should be active or passive
         */	

        CDRSource(const std::string &name, bool active)
        :Block(name, true), //ignore options, sniffer must be active
        m_source(NULL),
	 //        m_live(false), 
        m_gate_id(register_output_gate("sniffer_out")),
        m_mem_block(new MemoryBatch(4096*4))
        {
            if (!active) {
                blocklog("CDRSource must be active, ignoring configuration", log_warning);
            }
        }

        /**
         * @brief Destructor
         */
        virtual ~CDRSource()
        {
		/*
		std::cout<<"\n\nPkt count Sniffer: "<<pkt_count<<"\n";
		std::cout<<"Pkt count Layer  : "<<g_pkt_count<<"\n\n\n";

            if(m_source)
                pcap_close(m_source);
		*/
        }


        /**
         * @brief Configures the block: defines the capture interface, whether it's a trace or a live socket and (optional filters)
         * also, it opens the socket for capturing
         * @param n The configuration parameters 
         */
        virtual void _configure(const xml_node& n)
        {
            xml_node source = n.child("source");

            if (!source) 
                throw std::runtime_error("cdrsource: missing parameter source");

            std::string name=source.attribute("name").value();

            if(name.length()==0)
                throw std::runtime_error("cdrsource: missing attribute");

	    /* here try open the file */

	    m_source = new std::ifstream(name.c_str(), std::ifstream::in);

	    if(m_source->fail())
	    {
                throw(std::invalid_argument("CDRSource: error within ifstream"));
	    }
        }

        /** 
          * listens on the capture socket, retrieves packets and sends them out as Packet messages
          * Uses optimized allocation.
          * As the socket call is blocking (although it returns after a timeout expires) this cannot share a thread
          */
        void _do_async_processing()
        {

	  char Buff[256];
	  
	  while ( !m_source->getline( Buff, 256 ).eof() )
	  {	   
	    send_out_through(alloc_msg_from_buffer<CDRMessage>(m_mem_block, 0, Buff), m_gate_id);
	  }


	  throw(std::invalid_argument("Trace is over"));

	  /*
            struct pcap_pkthdr* hdr=NULL;
            const u_char* pkt = NULL;
            int ret_code = pcap_next_ex(m_source,&hdr,&pkt);

            while (ret_code == 1)
            {
                timespec p_tstamp;
                p_tstamp.tv_sec = hdr->ts.tv_sec;
                p_tstamp.tv_nsec = hdr->ts.tv_usec*1000;

		pkt_count++;
                send_out_through(alloc_msg_from_buffer<CDRMessage>(m_mem_block, (size_t)(hdr->caplen), const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(pkt), hdr->caplen), p_tstamp, hdr->len ), m_gate_id);
		
		
		//Packet* packet = new Packet(m_mem_block, (size_t)(hdr->caplen), const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(pkt), hdr->caplen), p_tstamp, hdr->len );
		//std::vector<std::vector<std::string>> mat;
		//std::shared_ptr <CounterIP_S> n (new CounterIP_S(alloc_msg_from_buffer<Packet> (m_mem_block, (size_t)(hdr->caplen), const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(pkt), hdr->caplen), p_tstamp, hdr->len ), mat));
		//send_out_through(std::move(n), m_gate_id);
		

                ret_code = pcap_next_ex(m_source, &hdr, &pkt);
            }

            if (ret_code == -1)
            {
                blocklog(std::string(pcap_geterr(m_source)), log_warning);
            } else if(ret_code == 0) {
                //timeout expired
		
            } else if(ret_code == -2) {
                //blocklog("trace is over", log_info);
			std::cout<<"\n\nPkt count Sniffer: "<<pkt_count<<"\n";
			std::cout<<"Pkt count Layer  : "<<g_pkt_count<<"\n\n\n";
			throw(std::invalid_argument("Trace is over"));

            }
	  */
        }

    };

    REGISTER_BLOCK(CDRSource,"CDRSource");
}
