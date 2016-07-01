
#pragma once

#include "IBuffer.hpp"
#include "Thread_lock.hpp"

class ICounter
{

protected:

	Thread_lock* counterLock;

	ICounter() : counterLock(NULL) {}

public:

    virtual double get(IReadBuffer& key, uint64_t args = 0) = 0;

    virtual double add(IReadBuffer& key, double value, uint64_t args = 0) = 0;
    
    virtual void clear() = 0;

    virtual bool reset(IReadBuffer& key)
    {
        return false;
    }

    static ICounter* getInstance(std::string type, int nhash, int shash, double beta);

    
    virtual ~ICounter()
    {
		if (counterLock) delete counterLock;
    }

};
