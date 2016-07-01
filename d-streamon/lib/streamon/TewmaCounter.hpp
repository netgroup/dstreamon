
#pragma once

#include "ICounter.hpp"
#include "HashSplitter.hpp"

#include <cstring>
#include <cmath>

#include "Thread_lock.hpp"

class TewmaCounter : public ICounter
{

    // specific variables for the rmeterfp class
    time_t*	t;		// timestamp array
    double*	c;		// counter array

    // slots

    uint32_t slots;

	// smoothing parameter
    double	beta;
    double	logbeta;

	// time unit (in seconds)
    double	W;

    int nhash;
    int shash;

public:

    // basic functions (constructor, destructor, etc)

    TewmaCounter(int _nhash, int _shash, double _beta, double _w) : t(NULL),
                                                                    c(NULL),
                                                                    slots( 1 << _shash ),
                                                                    beta(_beta),
                                                                    logbeta( -log(beta) ),
                                                                    W(_w),
                                                                    nhash(_nhash),
                                                                    shash(_shash)
                                                                    
    {
        assert(_shash > 0 && _shash < 32);
        assert( _beta > 0 && _beta < 1 );
        assert( _w > 0 );

		counterLock = Thread_lock::init_lock();

        t = new time_t[slots]();
        c = new double[slots]();

        // clear();
    }

	virtual ~TewmaCounter()
    {
        if (t) delete[] t;
        if (c) delete[] c;
    }

	virtual void clear( /* time_t timestamp_start */);

    virtual double get(IReadBuffer& key, uint64_t args = 0);

    virtual double add(IReadBuffer& key, double quantity = 1, uint64_t args = 0);

	// void	set_beta(double b);

};
