/**
 * NO! NO! NO! NO! this is Flow.hpp
 *
 * this message provides a the information regarding a flow
 * this includes start and end time, packets and bytes count, the flow five-tuple as well as a buffer with reassembled data
 *
 * Copyright notice goes here.
 */
#ifndef __FLOW_H_
#define __FLOW_H_


#include "Msg.hpp"
#include "ClassId.hpp"
#include "NetTypes.hpp"
#include "Packet.hpp"

#include <ctime>
#include <cstring>

namespace bm
{

    class Flow: public Msg {

    public:
        /**
         * regular constructor          */
        Flow(const FlowKey& key):
        Msg(MSG_ID(Flow)),
        m_stime(),
        m_etime(),
        m_key(key),
        m_bytes(0),
        m_packets(0)
        {}

        const FlowKey& key() const {
            return m_key;
        }

        ustime_t start_time() const
        {
            return m_stime;
        }
        
        ustime_t end_time() const
        {
            return m_etime;
        }
        
        void expand_interval(ustime_t us) {
            if (m_stime == 0 || m_stime > us) m_stime = us;
            if (m_stime == 0 || m_etime < us) m_etime = us;
        }

        uint64_t bytes() const
        {
            return m_bytes;
        }

        void increment_bytes(uint64_t val) {
            m_bytes += val;
        }

        uint64_t packets() const
        {
            return m_packets;
        }

        void increment_packets(uint64_t val) {
            m_packets += val;
        }
          
        bool matches(const Packet& p) {
            return m_key == p.key();
        }
        
        bool matches(const Flow& f) {
            return m_key == f.key();
        }
        
        /*
         * destructor
         */
        virtual ~Flow()
        {}

        struct pod_struct
        {
            ustime_t stime;
            ustime_t etime;
            uint64_t bytes;
            uint64_t packets;
            uint32_t src_ip4;
            uint32_t dst_ip4;
            uint16_t src_port;
            uint16_t dst_port;
            uint8_t protol;
        };


        virtual int to_pod(const mutable_buffer<char>& inb)
        {
#warning Brian some of the fields here do not match, or did I miss something
          //  pod_struct* p=(pod_struct*) inb.addr();
          //  p->tuple=m_tuple;
          //  p->stime=m_stime;
          //  p->etime=m_etime;
          //  p->buffer=m_buffer.addr();
          //  p->buf_len=m_buffer.len();
          //  p->bytes=m_bytes;
          //  p->packets=m_packets;
          //  return sizeof(pod_struct);
            return 0;
        }

        /**
          *returns a copy of the message (which always owns its buffer)
          */

        virtual std::shared_ptr<Msg> clone() const
        {
            Flow* new_flow = new Flow(m_key);
            new_flow->m_etime = m_etime;
            new_flow->m_stime = m_stime;
            new_flow->m_bytes = m_bytes;
            new_flow->m_packets = m_packets;
            return std::shared_ptr<Msg>(new_flow);
        }

    protected:
        ustime_t m_stime;
        ustime_t m_etime;
        FlowKey  m_key;
        uint64_t m_bytes;
        uint64_t m_packets;
    };
}











#endif// __FLOW_H_
