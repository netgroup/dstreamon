

#ifndef _CDRMESSAGE_HPP
#define _CDRMESSAGE_HPP


#include "MonstreamMessage.hpp"

#define CALL_SRC 0
#define CALL_DST 1
#define CALL_DUR 2
#define CALL_RES 3

#define SIZE_CID 32

#define DELIM "\t"

namespace bm
{

  class CDRMessage : public MonstreamMessage
  {

    uint64_t timestamp;

  public:
  
    CDRMessage(memory_not_owned_t         owned_flag,
	       size_t                     buf_displace,
	       char* cdr) : MonstreamMessage(MSG_ID(CDRMessage))
    {
      char* cur = strtok(cdr, DELIM);
      this->timestamp = strtoul(cur, NULL, 10) / 1000;

      
      cur = strtok(NULL, DELIM);
      Tags->Map[ CALL_SRC ].append( cur, strlen(cur) );

      
      cur = strtok(NULL, DELIM);
      Tags->Map[ CALL_DST ].append( cur, strlen(cur) ); 

      
      cur = strtok(NULL, DELIM);
      double duration = strtod(cur, NULL) / 1000.0;
      Tags->Map[ CALL_DUR ] << duration;

      
      cur = strtok(NULL, DELIM);
      uint32_t estabilished = (cur[0] == '2');
      Tags->Map[ CALL_RES ] << estabilished;
    }

    virtual uint64_t timestamp_us() const
    {
      return timestamp;
    }

    
    virtual ~CDRMessage() { }

  };

}


#endif
