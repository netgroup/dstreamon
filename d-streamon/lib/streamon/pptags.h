
#ifndef _PPTAGS_H
#define _PPTAGS_H

#include <stdint.h>
#include <stdio.h>

#include <streamon/Streamon.hpp>

#include "../lib_botstream/Timeout.hpp"
#include <message/Msg.hpp>


#define DROP_PACKET     0x0
#define EXPORT_PACKET   0x1
#define TIMEOUT_SETTED  0x2
#define TIMEOUT_REMOVED 0x4
#define CUSTOM_ACTION   0x8

#define REGISTER_SIZE 256


typedef double(*feature_function)(double*);

typedef bool(*condition_function)(double*, const struct pptags*);

typedef int (*action_function)(const struct pptags*);

typedef bool(*matcher_function)(const pptags*);


struct state_data
{
  int Id;
  uint64_t Expire;
};


typedef struct
{    
    long MetricID     : 6;
    long VDFlowkeyGet : 6;
    long VDFlowkeySet : 6;
    long VMFlowkeySet : 6;
    long VMFlowkeyGet : 6;
    long _NotUsed     : 2;
   
} metric_op;


typedef struct
{
    long Id;
    feature_function Feature;

} feature;


typedef struct
{
  condition_function Condition;
  action_function* Action;
  int NextState;

} condition_tuple;


typedef struct
{
    int Id; 
    metric_op* MetricSet;
    feature* FeatureList;
    condition_tuple* ConditionList;
    action_function* PostCondActionList;
    uint32_t Timeout;
    int TimeoutState;

} state_tuple;


typedef struct
{
    matcher_function Match;
    state_tuple* StateTable;

    struct
    {
        int32_t TableId;
        int32_t RegId;
        
    } KeyPair;

    char AttackType;

} event;


enum ppfield
{
  IP_SRC_F,
  IP_DST_F,
  SRC_PORT_F,
  DST_PORT_F,
  PROTOCOL_F,
  FLAGS_F,
  DOMAIN_NAME_F,
  HTTP_URL_F = 6,
  MODBUS_TRANSACTION_IDENTIFIER_F = 6,
  DOMAIN_ADDR_F,
  HTTP_METHOD_F = 7,
  MODBUS_FUNCTION_CODE_F = 7,
  PACKET_NO_F,
  IP_LEN_F,
  HTTP_HOST_F,
  DOMAIN_SECOND_LEVEL_F = 10,
 
  /* METRICS_KEY_F, */
  /* DNS_RESOLVED_IP, */

  NUM_OF_PP_FIELDS /* this field must be the last one */
};

struct reset_action
{
    uint16_t VDReset;
    uint16_t VMReset;
    HashedBuffer<REGISTER_SIZE> VMResetKey;

    reset_action() : VDReset(0), VMReset(0)
    {
    }
};

struct pptags
{
  HashedBuffer<REGISTER_SIZE>* StateFlowkey;
  state_tuple* State;
  state_data* StateInfo; // int StateID;

  metric_op* MetricOps[16];
  double Metrics[16];
  double Features[16];
  mutable HashedBuffer<REGISTER_SIZE> Map[16];
  // IDictionary<std::string>** Tables;
  TimeoutManager* TManager;
  std::shared_ptr< const bm::Msg > Message;
  int32_t TEventId;

  mutable std::string export_channel;
  mutable std::string export_message;

  mutable std::map<std::string, std::string> zmq_publish;

  mutable struct
  {
     char Id       : 6;
     char Suspect  : 2;

  } Attack;

  struct reset_action* ResetMetrics;

  pptags() : StateFlowkey(NULL), State(NULL), StateInfo(NULL),
             TManager(NULL), Message(), TEventId(-1), export_channel(),
             export_message()
  {
      memset(MetricOps, 0, 16*sizeof(metric_op*));

      Attack.Id = 0;
      Attack.Suspect = 0;
  }

  /* Map can be used as placeholder,
     in order to allocate the exact 
     number of flowkeys */
};


typedef struct
{
  int Id;
  uint32_t Value;
  
} req_tuple;

#endif
