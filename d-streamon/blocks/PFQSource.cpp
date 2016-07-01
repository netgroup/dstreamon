/* Copyright (c) 2011, Consorzio Nazionale Interuniversitario 
 * per le Telecomunicazioni. 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of NEC Europe Ltd, Consorzio Nazionale 
 *      Interuniversitario per le Telecomunicazioni, Institut Telecom/Telecom 
 *      Bretagne, ETH Zürich, INVEA-TECH a.s. nor the names of its contributors 
 *      may be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT 
 * HOLDERBE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

/*
 * <blockinfo type="PFQSource" invocation="indirect" thread_exclusive="True">
 *   <humandesc>
 *   Wrapper to the PFQ capturing engine:
 *
 *   http://netgroup.iet.unipi.it/software/pfq/
 *
 *   A block of this kind can be associated to a network interface or to a
 *   subset of its associated hardware queues. This is specified as a
 *   configuration parameter: a set of queues for the interface can be
 *   specified, if this is empty the block captures all of the packets on the interface.
 *   This block supports batch message allocation in order to optimize performance 
 *   </humandesc>
 *
 *   <shortdesc>Captures traffic from a local interface by means of the PFQ capturing engine</shortdesc>
 *
 *   <gates>
 *     <gate type="output" name="source_out" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      element queues {
 *          attribute device {text}
 *          element queue {
 *               attribute number {xsd:integer}
 *               }*
 *      }
 *    }
 *   </paramsschema>
 *
 *   <paramsexample>
 *  <params>                                                                                                                                 
 *      <queues device="eth3">
 *          <queue number="0"/>
 *      </queues> 
 *  </params>
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */
#include <Block.hpp>
#include <Packet.hpp>
// #include <NewPacket.hpp>
#include <Buffer.hpp>

#include <pugixml.hpp>
#include <BlockFactory.hpp>

#include <pfq/pfq.hpp>
#include <mutex>

#include <PacketPlus.hpp>
#include<signal.h>

using namespace pugi;

namespace bm
{   

         
    /**
     * Implement a block that captures packets from PFQ socket (a Linux packet capture system) and injects them into BM as Packet messages
     */
   // static int counter;
   static std::atomic<int> overallPackets;

    void handler(int signum)
    {
        // std::cout<<totalPacket<<std::endl;
        exit(-1);
    }
    
    class PFQSource: public Block
    {

		void print_stats()
		{
			std::cout <<"\n\nTotal packets: "<< pktCounter <<"\n";
			std::cout << "Filtered packets: " << filtered_packets << "\n";
			std::cout << "Bad packets     : " << bad_packets << "\n\n";
 
    		std::cout << "Overall packets: " << overallPackets << "\n\n";
		}

    public:

        /**
         * @brief Constructor
         * @param name         The name of the source block
         * @param invocation   Invocation type of the block (Indirect, Direct, Async)
         */
        PFQSource(const std::string &name, bool invocation) : Block(name, true),
        													  m_gate_id(register_output_gate("sniffer_out")),
															  pktCounter(0),
															  filtered_packets(0),
														      bad_packets(0),
        													  m_mem_block(new MemoryBatch(4096*4)),
        													  m_pfq(),
        													  m_device()
        {

			/*
        	struct sigaction sa;
        	memset(&sa,0,sizeof(sa));
        	sa.sa_handler=handler;
        	sigaction(SIGINT,&sa,NULL);
			*/
		
            if (invocation != true) {
                blocklog("PFQSource must be Async, ignoring configuration", log_warning);
            }

	        // counter=0;
	        // pktCounter=0;
            // totalPacket=0;
	        
        }

        virtual ~PFQSource()
        {
			/*
			counter+=pktCounter;
            std::cout << "Total number of packets in the block: " << pktCounter << "\n";
            std::cout << "Total number of packets: " << counter << "\n";
			*/

			print_stats();
        }
        
        /**
         * @brief Configures the block: defines the capture interface, the possible hw-queues in use, etc.
         * @param n The configuration parameters 
         */
        void _configure(const pugi::xml_node& n) 
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;

            int offset = 0;
            int slots  = 131072;
            int caplen = 1500;

            bool timestamp = false;

            pugi::xml_node queues = n.child("queues");
            if(!queues)
                throw std::runtime_error("PFQSource:: no queues node");
            
            m_device = std::string(queues.attribute("device").value());
            if(m_device.length()==0)
                throw std::runtime_error("PFQSource::no device attribute ");
            
            auto clattr  = queues.attribute("caplen");
            auto offattr = queues.attribute("offset");
            auto slotattr = queues.attribute("slots");

            auto tsattr = queues.attribute("tstamp");

            if (!clattr.empty())
                caplen = clattr.as_int();
           
            if (!offattr.empty())
                offset = offattr.as_int();

            if (!slotattr.empty())
                slots = slotattr.as_int();

            if (!tsattr.empty())
                timestamp = tsattr.as_bool();

            std::vector<unsigned int> vq;
            
            m_pfq.open(caplen, offset, slots);

            for (xml_node queue=queues.child("queue"); queue; queue=queue.next_sibling("queue"))
                vq.push_back(queue.attribute("number").as_uint());
            
            if(!vq.empty())
            {
                auto it = vq.begin();
                auto it_e = vq.end();
                for (;it!=it_e;++it)
                {
                    m_pfq.add_device(m_device.c_str(),*it);
                }
            }
            else
            {
                blocklog("PFQSource: no queues specified, sniffing on device ", log_info);
                m_pfq.add_device(m_device.c_str(), net::pfq::any_queue);
            } 
          
            std::cout << "PFQ: dev:" << m_device << " caplen:" << caplen << " tstamp:" << std::boolalpha << timestamp; 
            std::cout << " queues:";
            std::copy(vq.begin(), vq.end(), std::ostream_iterator<int>(std::cout, ","));
            std::cout << std::endl;

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_max_caplen = std::max(m_max_caplen, caplen);
            }

/*
            blockmon::dynamic_buffer::size_of(m_max_caplen);
*/


            
            m_pfq.caplen(caplen);
            m_pfq.toggle_time_stamp(timestamp);
            m_pfq.enable();         
            
        }

        virtual void _initialize()
        {
            std::cout << __PRETTY_FUNCTION__  << std::endl;
        }

        /** 
          * listens on the capture socket, retrieves a batch of packets and sends them out as Packet messages
          * Uses optimized allocation.
          */
        void _do_async_processing() 
        {
			timespec p_tstamp;
            p_tstamp.tv_sec = 0;
            p_tstamp.tv_nsec = 0;
            
            net::queue b = m_pfq.read(10 /* microseconds */);

            auto it_e = b.end();
            int i = 0;
            for (auto it = b.begin(); it != it_e; ++it, i++)
            {
                auto actual_payload = reinterpret_cast<uint8_t *>(it.data());
                pktCounter++;
				overallPackets++;
	            // totalPacket++;
				                
	/*

                printf("==== PACKET ARRIVED (QUEUE %d, LEN %u CAPLEN %u)====\n", i, it->len, it->caplen);
		
                for (int i=0; i < it->caplen; i++)
                {
                    printf("%02x ", actual_payload[i]);
                    
                    if ( i%32 == 0 ) printf("\n");
                }
                
                printf("\n==== PACKET END ====\n\n");
                */
    
                while (!it->commit)
                {}

               /* if (pktCounter % 1000000 == 0)
            		std::cout << "packet arrived\n";	*/

//std::cout <<pktCounter<<"\n";
                
                /*
                auto sp = alloc_msg_from_buffer<Packet>(m_mem_block, it->caplen, const_buffer<uint8_t>(actual_payload, it->caplen),
                                                        p_tstamp, (uint16_t)it->caplen );


                auto pkt = static_cast<const Packet*>( sp.get() );

                pkt->~Packet();
*/

				try
				{             
                	std::shared_ptr<const Msg>&& sp = alloc_msg_from_buffer<PacketPlus>(m_mem_block, it->caplen, const_buffer<uint8_t>(actual_payload, it->caplen), p_tstamp, (uint16_t)it->caplen );

					auto pktp = static_cast<const PacketPlus*>(sp.get());
				                
                	// ip fragmentation patch

                	if ( !pktp->is_first_fragment() )
                	{
						if (filtered_packets == 0)
						{
							std::cout << "Please see the packet #" << pktCounter << ", seems IP fragmented.\n";
						}

						filtered_packets++;

                    	pktp->~PacketPlus();
                	}
					else
					{
                		send_out_through(std::move(sp), m_gate_id);
					}
				}
				catch (std::runtime_error err)
				{
					bad_packets++;

					std::cout << "packet #" << pktCounter << ", exception: " << err.what() << "\n";
				}


                // send_out_through(std::move(sp), m_gate_id);
            }
        }

    private:

        int m_gate_id;
        int pktCounter;
		int filtered_packets;
		int bad_packets;

        std::shared_ptr<MemoryBatch> m_mem_block;  

        net::pfq m_pfq;
        std::string m_device;

        static std::mutex m_mutex;
        static int m_max_caplen;
 
    };

    std::mutex PFQSource::m_mutex;
    int PFQSource::m_max_caplen;
     

#ifndef _BLOCKMON_DOXYGEN_SKIP_
    REGISTER_BLOCK(PFQSource,"PFQSource");
#endif /* _BLOCKMON_DOXYGEN_SKIP_ */
}

