// FIXME brian Redocument this; change the API
// FIXME add on-demand parsing of IP header, ports, and TCP header fields
// don't use five-tuple anymore
// add IPv6 support (FUTURE)

/**
 * @file 
 *
 * This message represents a Packet.
 */
 
// FIXME determine whether timespec is the right way to go

/* Idem hack */
#ifndef _PACKET_HPP_
#define _PACKET_HPP_ 

#include "Msg.hpp"
#include "Buffer.hpp"
#include "MemoryBatch.hpp"
#include "ClassId.hpp"
#include "MsgRegistry.hpp"
#include "NetTypes.hpp"
#include <cstring>
#include <string>
#include <cassert>
#include <ctime>
#include <arpa/inet.h>
#include "MonstreamMessage.hpp"
#include "NullMsg.hpp"
#include <smmintrin.h>


namespace bm
{
    /**
     * BlockMon Message representing a Packet. Packet contains a raw packet
     * captured from the wire, and automatically parses ethernet, IP, TCP,
     * and UDP headers on demand to provide access to its fields.
     *
     * Packets may be sent to IPFIXExporter; at present, ethernet headers
     * will be removed and timestamps clamped to millisecond precision
     * to provide compatibility with capfix.
     */

  class Packet : public MonstreamMessage
    {

     // int count_bits_popcnt(const uint8_t* buffer, int len) const
     // {
     //     const int blocks = len/4;
     //     const int remainder = len%4;

     //     int sum = 0;

     //     const uint8_t* ptr = buffer;

     //     for (int i=0; i < blocks; i++)
     //     {
     //         uint32_t block = *(uint32_t*) ptr;

     //         sum += _mm_popcnt_u32(block);

     //         ptr += 4;
     //     }

     //     for (int i=0; i < remainder; i++)
     //     {
     //         sum += _mm_popcnt_u32(*ptr);
     //         ptr++;
     //     }

     //     return sum;
     // }
      
    public:

      static const int kEthernet = 1;
      static const int kRawIP = 2;

        /**
        *  Create a new Packet, copying the packet content from an existing 
        *  buffer to an internal buffer owned by the Packet.
        *
        *  The Packet's layer 2, 3, and 4 headers will be parsed on demand.
        *
        *  FIXME consider replacing the packet buffer here with a slab
        *        allocator.
        *
        *  @param  buf       const_buffer pointing to length and bounds of the 
        *                    buffer to copy the packet from.
        *  @param  timestamp time at which packet was captured. 
        *                    Optional; if omitted, use the current system time.
        *  @param  len       Total length of the packet. Optional, if omitted or
        *                    0, uses the length of the buffer.
        *  @param  mac_type  Encapsulation of the packet; Optional, 
        *                    default is kEthernet
        *  @param  msg_id    BlockMon message identifier; optional, defaults
        *                    to the message ID for the Packet class. Used
        *                    by derived classes of Packet; see the MSG_ID 
        *                    macro in ClassID.hpp.
        */        
        Packet(const const_buffer<uint8_t> &buf,
               timespec                   timestamp = timespec(), 
               size_t                     len =       0,
               int                        mac_type =  kEthernet,
               int                        msg_id =    0)
	  : MonstreamMessage((!msg_id)? MSG_ID(Packet): msg_id),
        // : Msg((!msg_id)? MSG_ID(Packet): msg_id), 
          m_buffer_owned(true),
          m_buffer(new uint8_t[buf.len()]),
          m_caplen(buf.len()),
          m_length(len),
          m_tstamp(timestamp),
          m_mactype(mac_type),
          m_iphdr_ptr(NULL),
          m_l4hdr_ptr(NULL),
          m_l4payload_ptr(NULL),
          m_machdr_parsed(false),
          m_iphdr_parsed(false),
          m_ports_parsed(false),
          m_l4hdr_parsed(false)
        {
            if (!m_length) m_length = buf.len();
            memcpy(const_cast<uint8_t*>(m_buffer) , buf.addr(), buf.len());
        }

        /**
        * Create a new Packet, using an external buffer allocated from 
        * within a MemoryBatch for the packet content storage. This
        * constructor must be used with placement-new, and requires the
        * memory in m_buffer to be contiguously allocated with this object.
        *
        * This constructor is used by alloc_msg_from_buffer() in 
        * MemoryBatch.hpp; Blocks should use that interface instead.
        *
        * FIXME Nicola: what about alignment?
        * 
        *  The Packet's layer 2, 3, and 4 headers will be parsed on demand.
        *
        *  @param  buf_displace offset from *this to the first byte of 
        *                    the packet buffer (should be sizeof(Packet)), unless
        *                    called from a derived class
        *  @param  buf       const_buffer pointing to length and bounds of the 
        *                    buffer to copy the packet from.
        *  @param  timestamp time at which packet was captured. 
        *                    Optional; if omitted, use the current system time.
        *  @param  len       Total length of the packet. Optional, if omitted or
        *                    0, uses the length of the buffer.
        *  @param  mac_type  Encapsulation of the packet; Optional, 
        *                    default is kEthernet
        *  @param  msg_id    BlockMon message identifier; optional, defaults
        *                    to the message ID for the Packet class. Used
        *                    by derived classes of Packet; see the MSG_ID 
        *                    macro in ClassID.hpp.
        * 
        */        
        Packet(memory_not_owned_t,
               size_t                     buf_displace,
               const const_buffer<uint8_t> &buf,
               timespec                   timestamp = timespec(), 
               size_t                     len =       0,
               int                        mac_type =  kEthernet,
               int                        msg_id = 0   )
	  : MonstreamMessage((!msg_id)? MSG_ID(Packet): msg_id),
        // : Msg((!msg_id)? MSG_ID(Packet): msg_id), 
          m_buffer_owned(false),
          m_buffer(reinterpret_cast<uint8_t *>(this) + buf_displace), 
          m_caplen(buf.len()),
          m_length(len),
          m_tstamp(timestamp),
          m_mactype(mac_type),
          m_iphdr_ptr(NULL),
          m_l4hdr_ptr(NULL),
          m_l4payload_ptr(NULL),
          m_machdr_parsed(false),
          m_iphdr_parsed(false),
          m_ports_parsed(false),
          m_l4hdr_parsed(false)
      {

            if (!m_length) m_length = buf.len();
            memcpy(const_cast<uint8_t*>(m_buffer), buf.addr(), buf.len());
        }

        /**
        * No copy constructor: Packets are not copyable.
        */
        Packet(const Packet &) = delete;
        
        /**
        * No copy assignment operator: Packets are not copyable.
        */
        Packet& operator=(const Packet &) = delete;

        /**
        * No move constructor: Packets are not movable.
        */
        Packet(Packet &&) = delete;
        
        /**
        * No move assignment operator: Packets are not movable.
        */
        Packet& operator=(Packet &&) = delete;

        /**
        * Destroy this Packet. Frees the packet buffer if owned.
        */
        virtual ~Packet()
        {
            if (m_buffer_owned) {
              delete m_buffer;
            }
        }

        /**
         * Clone this message; the new Packet will have the same content, own
         * its buffer, and will lose its parsed field cache.
         */
        virtual std::shared_ptr<Msg> clone() const
        {
            return std::make_shared<Packet>(
                const_buffer<uint8_t>(m_buffer, m_caplen), 
                m_tstamp, m_length, m_mactype);
        }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Parsers (currently under construction)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    public:
        /** Runt packet missing full layer 2 header */
        static const uint16_t  kPktTypeRuntL2 = 0xFFF2;
        /** Runt IP packet missing full layer 3 header */
        static const uint16_t  kPktTypeRuntL3 = 0xFFF3;
        /** Runt IP packet missing full layer 4 header */
        static const uint16_t  kPktTypeRuntL4 = 0xFFF4;
        
        /** IPv4 packet */
        static const uint16_t  kPktTypeIP4 = 0x0800;
        /** 802.1q packet; temporary packet type used during MAC parsing */        
        static const uint16_t  kPktType1q  = 0x8100;
        /** IPv6 packet. Not yet supported. */
        static const uint16_t  kPktTypeIP6 = 0x86FF;

        static const uint8_t   kUDP = 17;
        static const uint8_t   kTCP = 6;

        static const uint8_t   kFIN = 0x01;
        static const uint8_t   kSYN = 0x02;
        static const uint8_t   kRST = 0x04;
        static const uint8_t   kPSH = 0x08;
        static const uint8_t   kACK = 0x10;
        static const uint8_t   kURG = 0x20;
        static const uint8_t   kECE = 0x40;
        static const uint8_t   kCWR = 0x80;


    private:

        void parse_mac() const;
        void parse_iphdr() const;
        void parse_ports() const;
        void parse_tcphdr() const;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Standard accessors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

public:

        /** Get the packet's timestamp as a timespec */
        const timespec& timestamp() const {
            return m_tstamp;
        }


        virtual void timestamp_set(timespec& ts) const
        {
            m_tstamp = ts;
        }
        
        uint64_t timestamp_s() const {
            return m_tstamp.tv_sec;
        }

        uint64_t timestamp_ms() const {
            return timespec_to_us(m_tstamp) / 1000;
        }

        uint64_t timestamp_us() const {
            return timespec_to_us(m_tstamp);
        }

        /** Get the total length of the packet captured from the wire */
        size_t length() const {
            return m_length;
        }

        /** Get a pointer to the start of the packet */
        const uint8_t *base() const {
            return m_buffer;
        }

        /** Get a pointer to the start of the IP header;
         *  returns NULL for non-IP packets */
        const uint8_t *iphdr() const {
            parse_mac();
            return m_iphdr_ptr;
        }

        /** Get a pointer to the start of the L4 header;
         *  returns NULL for non-IP packets */
        const uint8_t *l4hdr() const {
            parse_iphdr();
            return m_l4hdr_ptr;
        }

        /** Get a pointer to the start of the L4 header;
         *  returns NULL for non-TCP/UDP packets */
        const uint8_t *l4payload() const {
            if (is_tcp()) {
                parse_tcphdr();
            } else {
                parse_ports();
            }
            return m_l4payload_ptr;
        }
        
        /** Get the MAC type of the packet; 
         *  presently supports ethernet and raw IP */
        int mac_type() const {
           return m_mactype;
        }
	//FIXME provisional by Andrea	
	uint16_t  l3_protocol() const
	{
	    parse_mac();
	    return m_pkttype;
	}

        uint32_t src_ip4() const {
           parse_iphdr();
           return m_key.src_ip4;
        }
   
        uint32_t dst_ip4() const {
           parse_iphdr();
           return m_key.dst_ip4;
        }

        uint16_t src_port() const {
           parse_ports();
           return m_key.src_port;
        }

        uint16_t dst_port() const {
           parse_ports();
           return m_key.dst_port;
        }

        uint8_t  protocol() const {
           parse_iphdr();
           return m_key.proto;
        }
        
        const FlowKey& key() const {
            parse_ports();
            return m_key;
        }

        bool is_tcp() const {
           return protocol() == kTCP;
        }

        bool is_udp() const {
            return protocol() == kUDP;
        }

        uint8_t  ip_tos() const {
           parse_iphdr();
           return m_iptos;
        }

        uint8_t  ip_ttl() const {
           parse_iphdr();
           return m_ipttl;
        }

        uint8_t tcp_flags() const {
           parse_tcphdr();
           return m_tcpflags;
        }

        bool tcp_has_syn() const {
           return tcp_flags() & kSYN;
        }
        
        bool tcp_has_ack() const {
           return tcp_flags() & kACK;
        }
        
        bool tcp_has_synack() const {
	      if(tcp_has_syn() && tcp_has_ack())
              return true;
	      else
	         return false;
        }        

        bool tcp_has_fin() const {
           return tcp_flags() & kFIN;
        }

        bool tcp_has_rst() const {
           return tcp_flags() & kRST;
        }

        bool tcp_has_psh() const {
           return tcp_flags() & kPSH;
        }

        bool tcp_ack(uint32_t& tcpack) const {
           if (tcp_flags() & kACK) {
               tcpack = m_tcpack;
               return true;
           } else {
               return false;
           }
        }

        bool tcp_urg(uint8_t& tcpurg) const {
           if (tcp_flags() & kURG) {
               tcpurg = m_tcpurg;
               return true;
           } else {
               return false;
           }
        }

        uint32_t tcp_seq() const {
          parse_tcphdr();
          return m_tcpseq;
        }

        uint32_t tcp_win() const {
          parse_tcphdr();
          return m_tcpwin;
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
      // double get_entropy() const
      // {
      //     int bits = count_bits_popcnt(m_buffer, m_length);

      //     double p = bits/(double)(m_length*8);

      //     return -(p*log(p) + (1-p)*log(1-p));
      // }


      // double get_p() const
      // {
      //     const int offset = 38;

      //     int bits = count_bits_popcnt(m_buffer+offset, m_length-offset);

      //     double p = bits/(double)((m_length-offset)*8);

      //     return p;
      // }


      double get_printable() const
      {
          const int offset = 38;

          int sum = 0;

          for (size_t i=offset; i < m_length; i++)
              sum += !!(isprint( m_buffer[i] ));


          return sum/double(m_length-offset);
      }
      


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Import/export via IPFIX
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
        struct pod_struct
        {
            uint64_t            ms;
            size_t              len;
            IPFIX::VarlenField  buf;
        }; 
        
        class Descriptor: public Msg::Descriptor
        {
        public:
            /**
             * Construct a new Packet from the POD representation returned by
             * copy_pod for the given Msg.
             *
             * @param buf buffer containing the POD representation
             */
            virtual const Msg& create_from_pod(const_buffer<uint8_t>& buf) {
                static NullMessage nm(0);
                return nm;

	      /*
                // check buffer size
                if (buf.len() < sizeof(pod_struct)) {
                    throw std::runtime_error("Buffer too small for create_from_pod");
                }
                
                const pod_struct* s = reinterpret_cast<const pod_struct*> (buf.addr());
                const_buffer<uint8_t> content_buf(s->buf.cp, s->buf.len);
                timespec ts;
                ts.tv_sec = s->ms / 1000;
                ts.tv_nsec = (s->ms % 1000) * 1000000;
                return Packet(content_buf, ts, s->len, kRawIP);
	      */
            }
        
            /**
             * Given a reference to an empty IETemplate, fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param model information model to look up IEs in
             * @param st template to fill
             */
            virtual void fill_pod_template (IPFIX::StructTemplate& st) const {
                IPFIX::InfoModel& model = IPFIX::InfoModel::instance();
                st.add(model.lookupIE("observationTimeMilliseconds"), offsetof(pod_struct, ms));
                st.add(model.lookupIE("ipTotalLength")->forLen(sizeof(size_t)), offsetof(pod_struct, len));
                st.add(model.lookupIE("ipHeaderPacketSection"), offsetof(pod_struct, buf));
                st.activate();
            }
        
            /**
             * Given a reference to an empty IETemplate (a WireTemplate returned
             * from IPFIX::Session.getTemplate()), fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param model information model to look up IEs in
             * @param wt template to fill
             */
            virtual void fill_wire_template (IPFIX::WireTemplate& wt) const {
                IPFIX::InfoModel& model = IPFIX::InfoModel::instance();
                wt.add(model.lookupIE("observationTimeMilliseconds"));
                wt.add(model.lookupIE("ipTotalLength[2]"));
                wt.add(model.lookupIE("ipHeaderPacketSection"));
            }
        };
                
        virtual void pod_copy(const mutable_buffer<char>& buf ) const
        {
            assert_pod_size(buf.len());
            
            pod_struct* s = (pod_struct*) buf.addr();
            s->ms = static_cast<uint64_t>(m_tstamp.tv_sec) * 1000 + 
                    static_cast<uint64_t>(m_tstamp.tv_nsec) / 1000000;
            s->len = m_length;
            // FIXME how to correctly handle constness with varlen fields?
            if (iphdr()) {
                s->buf.cp = const_cast<uint8_t*>(iphdr());
                s->buf.len = m_caplen;
            } else {
                // FIXME how to veto sending of non-IP packets for real,
                // or do we send MAC packets? for now, caplen is zero
                s->buf.cp = const_cast<uint8_t*>(base());
                s->buf.len = 0;
            }
        }
        
        virtual size_t pod_size() const
        {
            return sizeof(Packet::pod_struct);
        }

    protected:
        bool                  m_buffer_owned;
        const uint8_t*        m_buffer;
        size_t                m_caplen;
        size_t                m_length;
        mutable timespec      m_tstamp;
        int                   m_mactype;
        
        mutable const uint8_t* m_iphdr_ptr;
        mutable const uint8_t* m_l4hdr_ptr;
        mutable const uint8_t* m_l4payload_ptr;

        mutable bool          m_machdr_parsed;
        mutable bool          m_iphdr_parsed;
        mutable bool          m_ports_parsed;
        mutable bool          m_l4hdr_parsed;

        /** Packet type: mixture of ethertype and internal kPktType* 
            constants to signal what kind of packet we're dealing with */
        mutable uint16_t      m_pkttype;

        mutable FlowKey       m_key;
                              
        mutable uint8_t       m_iptos;
        mutable uint8_t       m_ipttl;
                              
        mutable uint8_t       m_tcpflags;
        mutable uint32_t      m_tcpseq;
        mutable uint32_t      m_tcpack;
        mutable uint32_t      m_tcpurg;
        mutable uint32_t      m_tcpwin;
    };

    REGISTER_MESSAGE_CLASS(Packet,Packet::Descriptor)

}


#endif /* _PACKET_HPP_ */
