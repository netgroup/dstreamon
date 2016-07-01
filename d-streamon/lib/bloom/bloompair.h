#ifndef _BLOOMPAIR_H
#define _BLOOMPAIR_H
#include "bloom.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
#include <math.h>
#include <streamon/Thread_lock.hpp>

class BloomPair : public khash {

	// specific variables for the Bloom class
    bloomfilter* 	B_learning;
    bloomfilter*	B_detecting;
    unsigned int	m; 		// bloom filter size
    double		threshold;	// adaptive threshold, double, easier to manage

    BloomPair** slave_place;

	Thread_lock* lock;

 protected:
    
	void            swap()
    {
		bloomfilter *b;
		b = B_detecting;

		unsigned dd = b->get_nzero();

		B_detecting = B_learning;
		B_learning = b;
		B_learning->clear();
		threshold = threshold * sqrt(double(m)/2/dd);

		if ( slave_place != NULL )
		{
			BloomPair* slave = (*slave_place);

			assert( slave );

			slave->swap();
		}
    }

    public:
    BloomPair(int _nhash, int _shash, BloomPair** _slave_place = NULL);

	            ~BloomPair();

	void	    clear();

	bool 	    add(unsigned char *in, int len);

	bool 	    check(unsigned char* element, int len);

	bool        set_bits(const unsigned* bits, int len = 0);

	bool        check_bits(const unsigned* bits, int len = 0);

	void 	    print(FILE* fp = stdout);

    };

#endif

