
#ifndef _MONSTREAM_MSG_H
#define _MONSTREAM_MSG_H

#include "Msg.hpp"

#include "MemoryBatch.hpp"
#include "ClassId.hpp"
#include "MsgRegistry.hpp"
#include "NullMsg.hpp"

#include <streamon/Streamon.hpp>
#include <streamon/pptags.h>

namespace bm
{
  class MonstreamMessage : public Msg
  {
  protected:

    struct pptags* Tags;

  public:

    MonstreamMessage(int msg_id) : Msg( msg_id==0 ? MSG_ID(MonstreamMessage) : msg_id )
    {
      Tags = new struct pptags();
    }


    struct pptags* getTags() const
    {
      return Tags;
    }

    HashedBuffer<REGISTER_SIZE>* getFlowkey(int id) const
    {
      return (id >= 0 ? &Tags->Map[id] : NULL);
    }


    /* this must be deprecated */
    /*
    uint32_t getValue(int key) const
    {
      return Tags->Map[key].GetUInt32();
    }
    */


    double getMetrics(int index) const
    {	
      return Tags->Metrics[index];
    }


    void setMetrics(int index, double value) const
    {    
      Tags->Metrics[index] = value;
    }


    double getFeature(int index) const
    {
      return Tags->Features[index];
    }


    void setFeature(int index, double value) const
    {
      Tags->Features[index] = value;
    }
	

    void getMetricsArray(double** a, int& len) const
    {		
      *a = Tags->Metrics;
      len = 16; // lenght;
    }


    void getFeaturesArray(double** a, int& len) const
    {
      *a = Tags->Features;
      len = 16; // lenght;
    }

    virtual uint64_t timestamp_us() const
    {
      return 0;
    }

    
    virtual void timestamp_set(timespec& ts) const
    {
    }


    virtual std::ostream& OutputWrite( std::ostream& Stream ) const
    {
      return Stream;
    }


    friend std::ostream& operator<<( std::ostream& Stream, const MonstreamMessage& msg )
    {
      return msg.OutputWrite( Stream );
    }


    virtual ~MonstreamMessage()
    {
      delete Tags;
    }


    class Descriptor : public Msg::Descriptor
    {
    public:

      virtual const Msg& create_from_pod(const_buffer<uint8_t>& buf)
      {
        static NullMessage nm(0);
        return nm;
      }
        

      virtual void fill_pod_template (IPFIX::StructTemplate& st) const
      {
      }
   
      virtual void fill_wire_template (IPFIX::WireTemplate& wt) const
      {
      }
    };


  };

  REGISTER_MESSAGE_CLASS(MonstreamMessage, MonstreamMessage::Descriptor)
}

#endif /* _MONSTREAM_MSG_H */
