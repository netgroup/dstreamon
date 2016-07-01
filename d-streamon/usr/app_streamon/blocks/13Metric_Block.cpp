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



#include "Metric_Block.hpp"
#include <streamon/ICounter.hpp>

namespace bm
{
    BloomPair* Metric_Block::det_instances[] = { NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL,
                                                 NULL, NULL, NULL, NULL, 
                                                 NULL, NULL, NULL, NULL };

	ICounter* Metric_Block::counter_instances[] = { NULL, NULL, NULL, NULL,
                                                   	NULL, NULL, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, 
                                                    NULL, NULL, NULL, NULL };



    struct reset_action Metric_Block::reset_ops[MAX_METRICS_NUM];


    void Metric_Block::_configure(const xml_node&  n )
    {
        xml_node bpf = n.child("block_type");

        if (bpf)
        {
            auto mid_attr = bpf.attribute("metric_id");

            std::string val = mid_attr.value();
      
            if (val != "null" && val != "")
            {
                metric_id = mid_attr.as_int();
            }
            else
                throw(std::invalid_argument("metric_id"));
					
        }

    
        bpf = n.child("variation_detector");

        if (bpf)
        {
            std::string val = bpf.attribute("status").value();

            if (val == "on")
                variation_detector = true;

            val = bpf.attribute("size").value();

            if (val != "null" && val != "")
                swap_bf_size = string_to_int(val);

            val = bpf.attribute("nhash").value();

            if (val != "null" && val != "")
                swap_bf_nhash = string_to_int(val);


            /*** TIMED RESET HANDLING ***/

            auto val_attr = bpf.attribute("reset_after");

            std::string val_s = val_attr.value();

            if (val_s != "")
            {
                swap_bf_reset_after = val_attr.as_int(); // * 1000000; // converting time in us
            }


            /*** MASTER/SLAVE HANDLING ***/

            int slave_id = -1;
            BloomPair** bloom_slave = NULL;


            auto slave_node = bpf.attribute("slave_id");

            std::string slave_node_s = slave_node.value();


            if (slave_node_s != "")
            {
                slave_id = slave_node.as_int();

                if (slave_id >= 0 and slave_id < MAX_METRICS_NUM)
                {
                    bloom_slave = &( Metric_Block::det_instances[ slave_id ] );

                    std::cout << "BF " << slave_id << " slave of metric " << metric_id << "\n";
                }
                else
                    throw std::invalid_argument("slave_id");	
            }

			if (Metric_Block::det_instances[metric_id] == NULL)
			{
				Metric_Block::det_instances[metric_id] = new BloomPair(swap_bf_nhash, swap_bf_size, bloom_slave);
			}

            bloom = Metric_Block::det_instances[metric_id];
		}


        bpf = n.child("variation_monitor");

        if (bpf)
        {
            // vars defaults

            int nhash       = 8;
            int shash       = 20;
            double life     = 1;

            double time_window = 60;
            double beta        = 0;


            // parse config

            std::string status        = bpf.attribute("status").value();
            std::string counter_type  = bpf.attribute("type").value();
      

            if (status == "on")
            {
                variation_monitor = true;
                
                if (counter_type == "")
                {
                    throw std::runtime_error("[MetricBlock] VM type must be specified");
                }
            }


            // Try to get config info, if not present roll-back to the default

            shash       = bpf.attribute("size").as_int()            ?: shash;
            nhash       = bpf.attribute("nhash").as_int()           ?: nhash;
            time_window = bpf.attribute("time_window").as_double()  ?: time_window;
            life        = bpf.attribute("life").as_double()         ?: life; 

            IndexValue  = bpf.attribute("index_value").as_int()     ?: -1;
            InsertValue = bpf.attribute("insert_value").as_int()    ?: 1;

            assert(InsertValue > 0);

            // beta = (1-2/(((3600/time_window)*life)+1)); // THIS IS WRONG!
       
            /* Note: time_window is the number of seconds of memory 
             * BETA will be computed as following: 
             */

            beta = 1 - 2/((time_window/6)+1);

            // time_window is scaled by a factor 6, such that time_window can be 
            // expressed in seconds.
            // Without scaling time_window is the number of 6-seconds windows. 

            // Get a counter instance based on type

			if (Metric_Block::counter_instances[metric_id] == NULL)
			{
				Metric_Block::counter_instances[metric_id] = ICounter::getInstance(counter_type, nhash, shash, beta);
			}

			counter = Metric_Block::counter_instances[metric_id];

            if (counter == NULL)
            {
                throw std::runtime_error("[MetricBlock] VM type wrong, candidates are: tewma, dleft, simple");
            }

            // tewma_bloom = new TEWMA(tewma_bf_nhash, tewma_bf_size, beta, time_window);
        }
    }


  void Metric_Block::_receive_msg(std::shared_ptr<const Msg>&& m, int /* index */)
  {			
    pkt_count++;
    
    auto pktp = static_cast<const MonstreamMessage*>(m.get());

    pptags* Tags = pktp->getTags();

    metric_op* Operation = Tags->MetricOps[metric_id];

    if (Operation == NULL)
    {
      send_out_through(std::move(m), id_outgate);
      return;
    }
    
    HashedBuffer<REGISTER_SIZE>* DetectorGetFlowkey = pktp->getFlowkey(Operation->VDFlowkeyGet);
    HashedBuffer<REGISTER_SIZE>* DetectorSetFlowkey = pktp->getFlowkey(Operation->VDFlowkeySet);
    
    HashedBuffer<REGISTER_SIZE>* MonitorSetFlowkey = pktp->getFlowkey(Operation->VMFlowkeySet);
    HashedBuffer<REGISTER_SIZE>* MonitorGetFlowkey = pktp->getFlowkey(Operation->VMFlowkeyGet);
    
    
    uint64_t time_packet = round(pktp->timestamp_us()/1000000.0);
	//std::cerr << "time_packet: " << time_packet << "\ttag: " << Tags->TEventId <<   std::endl;

 
    double Value = 0;

    bool isFlowNew = true;

    if (variation_detector == false && variation_monitor == false)
    {
        // Treat this as mov operation from Register to Metric Bank

        Value = MonitorGetFlowkey->Get<double>();

        goto update;
    }
    
    if (variation_detector == true)
    {
        if (reset_ops[metric_id].VDReset)
        {
            // reset bf
    
            bloom->clear();            

            reset_ops[metric_id].VDReset = 0;
        }

        if (swap_bf_reset_after > 0 and time_packet > reset_window_end)
        {
            /* RESET DETECTOR BF */

            bloom->clear();

            reset_window_end = time_packet + swap_bf_reset_after;
        }


        if (DetectorGetFlowkey != NULL)
        {
            Value = bloom->check_bits( DetectorGetFlowkey->GetHash().value()+1 );
	
            if (Value == 1)
                isFlowNew = false;
        }
        else
        {
            isFlowNew = false;
        }
      
      
        if (DetectorSetFlowkey != NULL)
        {
            bloom->set_bits( DetectorSetFlowkey->GetHash().value()+1 ); 
        }
    }

    
    if (variation_monitor == true)
    {        
        if (reset_ops[metric_id].VMReset)
        {
            HashedBuffer<REGISTER_SIZE>& reset_key = reset_ops[metric_id].VMResetKey;

            if (reset_key.GetLength() == 0)
            {
                counter->clear();
            }
            else
            {
                counter->reset(reset_key);

                reset_key.Reset();
            }

            reset_ops[metric_id].VMReset = 0;
        }

        if (isFlowNew == true && MonitorSetFlowkey != NULL)
        {
            if (IndexValue != -1)
                InsertValue = Tags->Map[IndexValue].Get<double>();

            Value = counter->add(*MonitorSetFlowkey, InsertValue, time_packet);

            // Value = tewma_bloom->set_bits(MonitorSetFlowkey->GetHash().value()+1, InsertValue, time_packet);
        }

      
        if (MonitorGetFlowkey != NULL) // && MonitorGetFlowkey != MonitorSetFlowkey)
        {
			//std::cout << Tags->Map[IP_SRC_F].GetString() << "\t" << Tags->Map[IP_DST_F].GetString();

            Value = counter->get(*MonitorGetFlowkey, time_packet);
			
            // Value = tewma_bloom->check_bits(MonitorGetFlowkey->GetHash().value()+1, time_packet);
        }
    }

update:    
    pktp->setMetrics(metric_id, Value);

    // std::cout << "\tmetric => " << Value << "\n";

    send_out_through(std::move(m), id_outgate);						
  }


  REGISTER_BLOCK(Metric_Block,"Metric_Block");
}

