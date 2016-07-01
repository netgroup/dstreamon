/**
 * <blockinfo type="Bloom_Block" is_active="False" thread_exclusive="False">
 *   <humandesc>
 *   Riceve un messaggio e Riempie i BloomFilter
 *   </humandesc>
 *
 *   <shortdesc>Filtro TCP</shortdesc>
 *
 *   <gates>
 *     <gate type="input" name="in_pkt_bloom" msg_type="Packet" m_start="0" m_end="0" />
 *     <gate type="output" name="out_bloom" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      }
 *   </paramsschema>
 *
 *   <paramsexample>
 *     <params>
 *     </params>
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */

#include <Block.hpp>
#include <BlockFactory.hpp>
#include <Packet.hpp>
#include <ClassId.hpp>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "bloom/bloompair.h"
#include "bloom/tewma.h"
#include "PacketPlus.hpp"
// #include "lib_botstream/Buffer.hpp"
#include <unordered_map>
#include <list>
#include <streamon/pptags.h>

#include "Metric_Block.hpp"

//#define PRINT_DEBUG

namespace bm
{
	class MetricSubnet : public Metric_Block
	{
		public:
			/*
			 * costruttore
			 */
			MetricSubnet(const std::string &name, bool): Metric_Block(name, false)
	                {
			  /*
			  id_ingate = register_input_gate("in_MetricSubnet");
			  id_outgate = register_output_gate("out_MetricSubnet");
			  */
			}

			MetricSubnet(const MetricSubnet &)			 = delete;				
			MetricSubnet& operator=(const MetricSubnet &)    = delete;		
			MetricSubnet(MetricSubnet &&)				 = delete;					
			MetricSubnet& operator=(MetricSubnet &&)		 = delete;			

			/*
			 * distruttore
			 */
			virtual ~MetricSubnet()
			{}
			
			
			/*
			 * il metodo principale
			 * riceve un messaggio PacketPlus, riempie i filtri e l'array con una metrica,
			 * se il messaggio appartiene ad un altro tipo viene catturata una eccezione.
			 */

			virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /* index */)
			{

			  /*
			   * Controllo per vedere se il pacchetto entrante è un packetPlus
			   */
			  if(m->type() != MSG_ID(PacketPlus))
			    throw std::runtime_error("wrong message type in metric_block");
			  
			  pkt_count++;

			  auto pktp = static_cast<const PacketPlus *>(m.get());

			  if ( !(pktp->is_udp() && pktp->src_port() == 53 ) ) // && pktp->get_DNSflag() == 0) )
			  {
			    send_out_through(std::move(m), id_outgate);

			    return;
			  }

			  buff_mon.Reset();

			  /*
			  auto packet = pktp->getPacket();
			  */

			  // uint64_t time_packet_us = pktp->timestamp_us();
			  // double time_packet = time_packet_us/1000000.0;

			  // std::vector<uint32_t> answersIP;
			  // pktp->get_answersIP(answersIP);

			  // std::string domain = pktp->get_domain_query();
			  // buff_mon << domain;
			  //   	
			  // for(unsigned int i = 0; i < answersIP.size(); i++)
			  // {
			  //   answersIP[i] = answersIP[i] & 0xFFFF0000;
			  // }					
			
	       
			  // for(unsigned int i=0; i<answersIP.size(); i++)
			  // {

			  //   buff_det.Reset();
			  //   buff_det << domain << answersIP[i];

			  //   if(!bloom->check( (unsigned char*) buff_det.GetRawData(), buff_det.GetLength()))
			  //   {			      
			  //     bloom->add((unsigned char*)buff_det.GetRawData(), buff_det.GetLength());
			  //     counter->add(buff_mon, 1, time_packet);
			  //   } 
			  // }

			  // pktp->setMetrics(metric_id, counter->get(buff_mon, time_packet));


			  send_out_through(std::move(m), id_outgate);						
			}
					
			
		private:

                        HashedBuffer<64> buff_mon;
                        HashedBuffer<64> buff_det;
	};

  

  BlockFactoryRegistration<MetricSubnet> __MetricSubnet_registration__("MetricSubnet");


  // REGISTER_BLOCK(MetricSubnet,"MetricSubnet");
}
