
#pragma once

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
#include <unordered_map>
#include <list>
#include <streamon/pptags.h>
#include <streamon/ICounter.hpp>

namespace bm
{
  // extern std::unordered_map<std::string, int> TokenMap;

  #define MAX_METRICS_NUM 16

  class Metric_Block : public Block
  {
  public:

    Metric_Block(const std::string &name, bool): Block(name, false),
						 id_ingate(register_input_gate("in_Metric_Block")),
						 id_outgate(register_output_gate("out_Metric_Block"))
    {

      variation_detector  = false;
      variation_monitor   = false;
      swap_bf_size	  = 16;
      swap_bf_nhash       = 4;
      swap_bf_reset_after = 0;
      tewma_bf_size       = 18;
      tewma_bf_nhash      = 4;
      time_window         = 60;
      life		  = 1;
      reset_window_end    = 0;
      
      IndexValue = -1;
      
      InsertValue = 1;								
    }

    
    Metric_Block(const Metric_Block &)		  = delete;
    Metric_Block& operator=(const Metric_Block &) = delete;
    Metric_Block(Metric_Block &&)		  = delete;
    Metric_Block& operator=(Metric_Block &&)	  = delete;
    
    virtual ~Metric_Block() {}

    /*
     * questo blocco ha parametri di configurazione
     */
    virtual void _configure(const xml_node&  n );

    virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /* index */);

    uint32_t string_to_int(std::string& val)
    {				
      uint32_t num;
      std::istringstream ss(val);
      ss >> num;
      return num;
    }


  protected:
    
    int id_ingate;
    int id_outgate;
    int pkt_count;
    int swap_bf_size;
    int swap_bf_nhash;
    uint64_t swap_bf_reset_after;
    int tewma_bf_size;
    int tewma_bf_nhash;
    int life;
    int metric_id;
    int isPresent;

    uint64_t reset_window_end;

    
    double beta;
    double time_window;
    
    std::string configuration;
    std::string det_flowkey;
    std::string mon_flowkey;

    bool variation_detector;
    bool variation_monitor;
    
    std::list<req_tuple> req_map;
    
    std::vector<int> var_detect_flowkey;
    std::vector<int> var_monitor_flowkey;

    double InsertValue;
    
    int IndexValue;
    
    BloomPair* bloom;

    // TEWMA* tewma_bloom;

    ICounter* counter;

    public:
    
    static BloomPair* det_instances[MAX_METRICS_NUM];

	static ICounter* counter_instances[MAX_METRICS_NUM];

    static struct reset_action reset_ops[MAX_METRICS_NUM];
  };
  
}
