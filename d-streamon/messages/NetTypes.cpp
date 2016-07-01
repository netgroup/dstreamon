#include "NetTypes.hpp"

namespace bm {

    static const uint64_t k10_3 = 1000;
    static const uint64_t k10_6 = 1000000;
    static const uint64_t k10_9 = 1000000000;
    static const uint64_t k2_32 = static_cast<double>(2ULL<<32);

    ustime_t timespec_to_us(const timespec& ts) {
        return ts.tv_sec * k10_6 + ts.tv_nsec / k10_3;
    }
    
    const timespec us_to_timespec (ustime_t us) {
        timespec ts;
        ts.tv_sec = us / k10_6;
        ts.tv_nsec = (us % k10_6) * k10_3;
        return ts;
    }
    
    const ntptime us_to_ntp (ustime_t us) {
        ntptime nt;
        nt.sec = us / k10_6;
        nt.frac = static_cast<uint32_t>(static_cast<double>(us % k10_6) / k10_6 * k2_32);
        return nt;
    }

    const ntptime timespec_to_ntp (const timespec& ts) {
        ntptime nt;
        nt.sec = ts.tv_sec;
        nt.frac = static_cast<uint32_t>(static_cast<double>(ts.tv_nsec) / k10_9 * k2_32);
        return nt;
    }

    ustime_t ntp_to_us (const ntptime& nt) {
        return nt.sec * k10_6 + static_cast<uint32_t>(static_cast<double>(nt.frac) * k10_6 / k2_32);
    }
    
    const timespec ntp_to_timespec(const ntptime& nt) {
        timespec ts;
        ts.tv_sec = nt.sec;
        ts.tv_nsec = static_cast<uint32_t>(static_cast<double>(nt.frac) * k10_9 / k2_32);
        return ts;
    }
}
