/**
 * <blockinfo type="PFRingSource" scheduling_type="True" thread_exclusive="True">
 *   <humandesc>
 *     Wrapper to the PFRing capturing engine (https://svn.ntop.org/svn/ntop/trunk/PF_RING/).
 *     This block can capture the traffic coming from a network interface or just of a hardware queue thereof.
 *     If no specific queues are specified in the configuration, the whole interface is monitored.
 *     In order for this block to compile, the PF_RING user space library needs to be installed (make install can do it).
 *     In order for this blocks to work, the PF_RING module has to be loaded beforehand.
 *   </humandesc>
 *
 *   <shortdesc>Wrapper to the PF_RING capturing engine </shortdesc>
 *
 *   <gates>
 *     <gate type="output" name="src_out" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      element queues {
 *        attribute device {text}
 *        element queue 
 *        {
 *             attribute number {xsd:integer}  
 *        }*
 *      }
 *      element bpf_filter {text}?
 *    }
 *   </paramsschema>
 *
 *   <paramsexample>
 *      <params>
 *           <queues device="eth0" >
 *           	<queue number="1"/>
 *           	<queue number="2"/>
 *           </queues>
 *      </params>
 *
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */


#ifdef PFRING_BLOCK


#include<algorithm>
#include<Block.hpp>
#include<Packet.hpp>
#include<BlockFactory.hpp>
#include<pfring.h>
#include<signal.h>



namespace bm
{
	extern int g_pkt_count;

	int s_pkt_count = 0;

	void SigHandle(int Sig)
	{
		std::cout << "\nSNIFFER PACKETS: " << s_pkt_count << "\n";
		std::cout << "\nFEATURE PACKETS: " << g_pkt_count << "\n";

		exit(-1);
	}

    class PFRingSource: public Block
    {
        int m_out_pkts;
        std::string m_device;
        std::vector<pfring*> m_ring;
        std::shared_ptr<MemoryBatch> m_mem_block;



        void create_enable_ring(std::string name)
        {
            int i=m_ring.size();//last position in vector
            m_ring.push_back(pfring_open(const_cast<char*>(name.c_str()), 1,  1500, 0));
            if(m_ring[i] == NULL) {
                throw std::runtime_error(std::string(name).append(" : impossible to open PFRING "));

            } else {
                char buf[128];
                snprintf(buf, sizeof(buf), "blockmon %s",name.c_str());
                pfring_set_application_name(m_ring[i], buf);
            }
            int rc;
            if((rc = pfring_set_direction(m_ring[i], rx_only_direction)) != 0)
            {
                char logmsg[256];
                sprintf(logmsg,"pfring_set_direction returned [rc=%d][direction=%d]\n", rc, 1);
                blocklog(std::string(logmsg), log_info);
            }
            pfring_enable_ring(m_ring[i]);


        }
    public:
        PFRingSource(const PFRingSource&) = delete;
        PFRingSource& operator=(const PFRingSource&) = delete;
        PFRingSource( PFRingSource&& ) = delete;
        PFRingSource& operator=(PFRingSource&&) = delete;

        PFRingSource(const std::string &name, bool active)
        : Block(name, active), 
        m_out_pkts(register_output_gate("sniffer_out")),
        m_mem_block(new MemoryBatch(4096*4))
        {}

        ~PFRingSource()
        {
            std::for_each(m_ring.begin(),m_ring.end(),pfring_close);
        }


        virtual void _configure(const xml_node&  n )
        {
            xml_node queues=n.child("queues");
            if(!queues)
                throw std::runtime_error("PFRingSource:: no queues node");
            m_device = std::string(queues.attribute("device").value());
            if(m_device.length() == 0)
                throw std::runtime_error("PFRingSource::no device attribute ");
            std::vector<unsigned int> vq;
            for (xml_node queue = queues.child("queue"); queue; queue = queue.next_sibling("queue"))
                vq.push_back(queue.attribute("number").as_uint());

            if(!vq.empty())
            {
                auto it = vq.begin();
                auto end = vq.end();
                for (; it != end ;++it)
                {
                    char devname[64];
                    snprintf(devname, sizeof(devname), "%s@%d", m_device.c_str(),*it);
                    create_enable_ring(std::string(devname));
                }

            }
            else
            {
                blocklog("no queues specified, sniffing on device", log_info);
                create_enable_ring(m_device);
            }
			struct sigaction SigAct;

			memset( &SigAct, 0, sizeof(SigAct) );

			SigAct.sa_handler = SigHandle;

			sigaction( SIGINT, &SigAct, NULL );
        }

        virtual void _do_async_processing()
        {

            for (int i=0; i<10000; ++i) {

                struct pfring_pkthdr hdr;
                auto it=m_ring.begin();
                auto end=m_ring.end();
                for (;it!=end;++it)
                {
                    u_char* p;
                    if(pfring_recv(*it,&p /*(char*)buffer*/, 0/*sizeof(buffer)*/, &hdr, 1) > 0)
                    {
                        timeval& tstamp = hdr.ts;
                        timespec tspec;
                        tspec.tv_sec = tstamp.tv_sec;
                        tspec.tv_nsec = tstamp.tv_usec * 1000;
			s_pkt_count++;
                        send_out_through(alloc_msg_from_buffer<Packet>(m_mem_block, hdr.len, const_buffer<unsigned char>(reinterpret_cast<const unsigned char *>(p)/*(buffer)*/,hdr.len), tspec, hdr.len ),m_out_pkts);

                    }
                }
            }
        }

    };


    REGISTER_BLOCK(PFRingSource,"PFRingSource");

};//bm

#else

#pragma message "PFRING block will not be compiled"

#endif


