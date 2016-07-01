
#pragma once

#include "ICounter.hpp"
#include "HashSplitter.hpp"
#include "DLeft.cpp"

#include <cstring>
#include <cmath>

class DLeftCounter : public ICounter
{

    DLeft<uint32_t>* table;

    
public:

    // basic functions (constructor, destructor, etc)

    DLeftCounter(int _nhash, int _shash)
    {
        assert(_shash > 0 && _shash < 32);

        table = new DLeft<uint32_t>(_nhash, _shash);

		counterLock = Thread_lock::init_lock();
    }

	virtual ~DLeftCounter()
    {
        if (table) delete table;
    }

	virtual void clear();

    virtual double get(IReadBuffer& key, uint64_t args = 0);

    virtual double add(IReadBuffer& key, double quantity = 1, uint64_t args = 0);

    virtual bool reset(IReadBuffer& key);

	// void	set_beta(double b);

};
