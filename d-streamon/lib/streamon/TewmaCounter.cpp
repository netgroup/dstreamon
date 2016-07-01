
#include "TewmaCounter.hpp"


double TewmaCounter::get(IReadBuffer& key, uint64_t args)
{
    HashSplitter splitter( key.GetHash(), 1 );

    time_t timestamp = (time_t) args;

    uint32_t H[9];

    for (int i=1; i <= nhash; i++)
    {
        assert( splitter.next(shash, &H[i]) );
    }

    // now computes the old accumulated value and the new one before this insertion

    counterLock->lock(false);

    double minctr = c[H[1]];

    int64_t deltat;

    for(int i=1; i<=nhash; i++)
    {
        deltat = timestamp - t[H[i]];
        assert(deltat>=0);

        t[H[i]] = timestamp;
        c[H[i]] *= pow(beta, double(deltat)/W);
        if (c[H[i]]<minctr) minctr=c[H[i]];

        // printf("hash no => %d, value => %u\n", i, H(i));
    }

	counterLock->unlock();

    // finally return value. 
    return minctr;
}


double TewmaCounter::add(IReadBuffer& key, double quantity, uint64_t args)
{
    HashSplitter splitter( key.GetHash(), 1 );

    time_t timestamp = (time_t) args;

    uint32_t H[9];

    for (int i=1; i <= nhash; i++)
    {
        assert( splitter.next(shash, &H[i]) );
    }


	counterLock->lock(true);

    // computes the new counter decayed value
    double minctr=c[H[1]];

    int64_t deltat;

    for(int i=1; i<=nhash; i++) {

        deltat = timestamp - t[H[i]];
        assert(deltat>=0);

        t[H[i]] = timestamp;

        c[H[i]] *= pow(beta, double(deltat)/W);
        if (c[H[i]]<minctr) minctr=c[H[i]];
    }

    // now compute the new value including the insertion and update 
    // the filter by waterfilling the new counter array bins
    double newctr=minctr+quantity*logbeta;
    assert(newctr<=1.0e99);

    for(int i=1; i<=nhash; i++) if (c[H[i]]<newctr) c[H[i]]=newctr;


	counterLock->unlock();
  
    // finally return value. 
    return newctr;

}

void TewmaCounter::clear()
{
    assert( t && c );

	counterLock->lock(true);

    memset(t, 0, slots * sizeof(time_t) );

    memset(c, 0, slots * sizeof(double) );

	counterLock->unlock();
}
