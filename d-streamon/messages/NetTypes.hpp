#ifndef _NETTYPES_HPP_
#define _NETTYPES_HPP_ 


#include <ctime>
#include <stdint.h>

namespace bm
{

struct ethhdr {
    uint8_t     smac[6];
    uint8_t     dmac[6];
    uint16_t    ethertype;
};

struct eth1qhdr {
    uint16_t        vlantag;
    uint16_t        ethertype;
};

struct ip4hdr {
    uint8_t         hlv;
    uint8_t         tos;
    uint16_t        len;
    uint16_t        fragid;
    uint16_t        fragoff;
    uint8_t         ttl;
    uint8_t         proto;
    uint16_t        cksum;
    uint32_t        sip4;
    uint32_t        dip4;
};

struct porthdr {
    uint16_t        sp;
    uint16_t        dp;
};

struct tcphdr {
    uint16_t        sp;
    uint16_t        dp;
    uint32_t        seqnum;
    uint32_t        acknum;
    uint8_t         payoff;
    uint8_t         flags;
    uint16_t        window;
    uint16_t        cksum;
    uint16_t        urgent;
};

struct udphdr {
    uint16_t        sp;
    uint16_t        dp;
    uint16_t        len;
    uint16_t        cksum;
};



    class FlowKey {
    public:
        uint32_t    src_ip4;
        uint32_t    dst_ip4;
        uint16_t    src_port;
        uint16_t    dst_port;
        uint8_t     proto;   

        FlowKey():
        src_ip4(0),
        dst_ip4(0),
        src_port(0),
        dst_port(0),
        proto(0) {}

        FlowKey(const FlowKey& rhs):
        src_ip4(rhs.src_ip4),
        dst_ip4(rhs.dst_ip4),
        src_port(rhs.src_port),
        dst_port(rhs.dst_port),
        proto(rhs.proto) {}

        FlowKey& operator=(const FlowKey &rhs) {
            if (this != &rhs) {
                src_ip4 = rhs.src_ip4;
                dst_ip4 = rhs.dst_ip4;
                src_port = rhs.src_port;
                dst_port = rhs.dst_port;
                proto = rhs.proto;
            }
            return *this;
        }

        bool operator== (const FlowKey& rhs) const {
            if ((src_ip4 == rhs.src_ip4) &&
                (dst_ip4 == rhs.dst_ip4) &&
                (src_port == rhs.src_port) &&
                (dst_port == rhs.dst_port) &&
                (proto == rhs.proto)) {
                return true;
            } else {
                return false;
            }
        }

        bool operator!= (const FlowKey& rhs) {
            return !(*this == rhs);
        } 
    };

    /** ustime_t is a timestamp in UNIX epoch milliseconds */
    typedef uint64_t ustime_t;
    
    /** ntptime_t is an NTP timestamp, in seconds since the NTP epoch, and fractional seconds */
    struct ntptime {
        uint32_t  sec;
        uint32_t  frac;
    };

    
    ustime_t timespec_to_us(const timespec& ts);
    const timespec us_to_timespec (ustime_t us);
    const ntptime us_to_ntp (ustime_t us);
    const ntptime timespec_to_ntp (const timespec& ts);
    ustime_t ntp_to_us (const ntptime& nt);
    const timespec ntp_to_timespec(const ntptime& nt);

}

#endif
