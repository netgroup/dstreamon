

/**
 * @file 
 *
 * This message represents a PacketPlus.
 */
 

/* Idem hack */
#ifndef _PacketPlus_HPP_
#define _PacketPlus_HPP_ 

#include "Msg.hpp"
#include "Buffer.hpp"
#include "MemoryBatch.hpp"
#include "ClassId.hpp"
#include "MsgRegistry.hpp"
#include "NetTypes.hpp"
#include "Packet.hpp"

#include "NullMsg.hpp"

#include <streamon/pptags.h>
#include <streamon/DnsHeader.hpp>
#include <streamon/DnsResponse.hpp>

#include <cstring>
#include <cassert>
#include <ctime>
#include "HttpRequest.hpp"
#include "modbus.hpp"

namespace bm
{
    /**
     * PacketPlus Msg: an ehnanced packet with extracted fields placed into an array
     */
  

    class PacketPlus : public Packet
    {
        std::string dns_second_level(const std::string& s)
        {
            int count = 0;

            auto it = s.crbegin();

            for (; it != s.crend(); ++it)
            {
                if (*it == '.') {
                    ++count;

                    if (count == 2)
                        break;
                }
            }

            size_t roffset = it - s.crbegin();

            size_t offset = s.size() - roffset;

            return s.substr(offset);
        }
      
    public:

    
        /**
         * Costruttori
         */
        PacketPlus(memory_not_owned_t         owned_flag,
                   size_t                     buf_displace,
                   const const_buffer<uint8_t> &buf,
                   timespec                   timestamp = timespec(), 
                   size_t                     len = 0 )

        : Packet(owned_flag, buf_displace, buf, timestamp, len, Packet::kEthernet, MSG_ID(PacketPlus))
        {
            m_pkt_count++;

            isValidDns = false;

            uint16_t s_port = src_port();
            uint16_t d_port = dst_port();

            Tags->Map[IP_SRC_F].append( src_ip4(), buffer_type::IPv4 );
            Tags->Map[IP_DST_F].append( dst_ip4(), buffer_type::IPv4 );

            Tags->Map[SRC_PORT_F] << s_port;
            Tags->Map[DST_PORT_F] << d_port;

            Tags->Map[PROTOCOL_F] << protocol();
      
            Tags->Map[IP_LEN_F] << (double) length();
      
            if ( is_tcp() )
            {
                Tags->Map[FLAGS_F] << tcp_flags();

				/*
				if ( dst_port() == 80 )
				{
					// printf("[PacketPlus] IP pointer: %p\n", iphdr());

					// printf("[PacketPlus] IP+TCP pointer: %p\n", iphdr() + sizeof(struct tcp));

					const uint8_t* pay = l4payload();

					printf("[PacketPlus] PAYLOAD pointer:\n%p\n\n", pay);
				}
				*/
				if ( d_port == 80 )
				{
					HttpRequest http( l4payload(), length() - 14 - sizeof(struct tcp) );
				
					if (http.parse())
					{
						Tags->Map[HTTP_METHOD_F] << (uint32_t) http.getMethod();
						Tags->Map[HTTP_URL_F] << http.getAbsoluteUri();
						Tags->Map[HTTP_HOST_F] << http.getHeader("Host");
					}
				}
				
				
				if ( d_port == 502 )
				{
					Modbus modb = Modbus::parse( l4payload(),  length() - 14 );
				
					if (modb.get_l() !=0)
					{
						Tags->Map[MODBUS_TRANSACTION_IDENTIFIER_F] <<  modb.get_ti();
						Tags->Map[MODBUS_FUNCTION_CODE_F] <<  modb.get_fc();
					}
				}

            }
            else
            if ( (d_port == 53 || s_port == 53) && is_udp() ) // DNS (Query or Response)
            {
                DnsHeader dnsh = DnsHeader::parse( iphdr(), length() - 14 );

                if (!!dnsh) // dns good
                {
                    Tags->Map[FLAGS_F] << dnsh.getReturnCode();

                    DnsResponse dnsr = DnsResponse::parse(dnsh); // this can throw an exception!

                    if (!!dnsr)
                    {
                        auto dns_alias = dnsr.getAlias();

                        Tags->Map[DOMAIN_NAME_F] << dns_alias;

                        Tags->Map[DOMAIN_SECOND_LEVEL_F] << dns_second_level(dns_alias);

                        if (dnsh.isAnswer()) {
                            auto ipList = dnsr.getAddresses();

                            if (ipList.size() > 0)
                            {
                                Tags->Map[DOMAIN_ADDR_F].append( ipList[0], buffer_type::IPv4 );
                            }
                        }

                        isValidDns = true;
                    }
                }
            }
            else
            {
                Tags->Map[ FLAGS_F ] << uint32_t(-1); 
            }

            Tags->Map[PACKET_NO_F] << m_pkt_count;
        }


        uint32_t packet_no() const
        {
            return m_pkt_count;
        }


        inline bool is_first_fragment() const
        {
            const uint8_t* iph = iphdr();

            if (!iph) return false;

            uint16_t flags_offset = ntohs(*reinterpret_cast<const uint16_t*>(iph+6));

            return (flags_offset & 0x1FFF) == 0; // take 13 bit offset
        }
    
	      
        virtual std::ostream& OutputWrite( std::ostream& Stream )
        {            
			Stream << Tags->Map[PROTOCOL_F].GetString() << "\t"
				   << Tags->Map[IP_SRC_F].GetString() << "\t"
				   << Tags->Map[IP_DST_F].GetString() << "\t"
				   << Tags->Map[SRC_PORT_F].GetString() << "\t"
				   << Tags->Map[DST_PORT_F].GetString();

            if (isValidDns)
                Stream << "\t" << Tags->Map[DOMAIN_NAME_F].GetString(); 
            Stream << "\n";

            return Stream;	
        }


		virtual ~PacketPlus()
        {
        }

    private:

        static uint32_t m_pkt_count;
		
		int m_type;

		bool isValidDns;
  };

  REGISTER_MESSAGE_CLASS(PacketPlus, Packet::Descriptor)
}


#endif /* _PacketPlus_HPP_ */
