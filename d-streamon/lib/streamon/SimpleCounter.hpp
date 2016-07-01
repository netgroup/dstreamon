
#pragma once

#include "ICounter.hpp"
#include <atomic>

class SimpleCounter : public ICounter
{

    std::atomic<uint32_t> count;

public:

    SimpleCounter() : count(0)
    {
    }

    
    double get(IReadBuffer& key, uint64_t args = 0)
    {
        return count;
    }

    double add(IReadBuffer& key, double value, uint64_t args = 0)
    {
        count += value;

        return count;
    }


    void clear()
    {
        count = 0;
    }
};
