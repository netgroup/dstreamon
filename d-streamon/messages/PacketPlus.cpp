#include "PacketPlus.hpp"
#include "NetTypes.hpp"
#include "Packet.hpp"
#include <netinet/in.h>
#include <unordered_map>

namespace bm {

    uint32_t PacketPlus::m_pkt_count = 0;


    std::string ip_to_string(uint32_t ip)
    {
        char addr_buffer[INET_ADDRSTRLEN];

        /*
         *inet_ntop expects network byte order
         */

        uint32_t flipped_ip=htonl(ip);

        if(!inet_ntop(AF_INET, &flipped_ip, addr_buffer, INET_ADDRSTRLEN))
        {
            throw std::runtime_error("cannot convert ip address");
        }

        return std::string (addr_buffer);
    }
}
