
#ifndef _NULL_MSG_H
#define _NULL_MSG_H

#include "Msg.hpp"

#include "Buffer.hpp"
#include "MemoryBatch.hpp"
#include "ClassId.hpp"
#include "MsgRegistry.hpp"

namespace bm
{
  class NullMessage : public Msg
  {
  public:

    NullMessage(int msg_id) : Msg( msg_id==0 ? MSG_ID(NullMessage) : msg_id )
    {
    }

    class Descriptor : public Msg::Descriptor
    {
    public:

      virtual const Msg& create_from_pod(const_buffer<uint8_t>& buf) {
        static NullMessage nm(0);
        return nm;
      }
        
      virtual void fill_pod_template (IPFIX::StructTemplate& st) const {
      }
   
      virtual void fill_wire_template (IPFIX::WireTemplate& wt) const {
      }
    };


  };

  REGISTER_MESSAGE_CLASS(NullMessage, NullMessage::Descriptor)
}

#endif /* _NULL_MSG_H */
