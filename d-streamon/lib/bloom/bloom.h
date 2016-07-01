#ifndef _BLOOM_H
#define _BLOOM_H
#include "khash.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

class bloomfilter : public khash {

	// specific variables for the Bloom class
    unsigned char*  m;		// bit array
    unsigned	    mmax;	// number of bits
    unsigned	    mmax_arr;	// bit array size (blocks of 8 bits)
    unsigned	    num_zeros;  // number of zeroes in the filter

	// variables inherited from khash
	//     	nhash;
	//	shash;


    public:
                    bloomfilter(int _nhash, int _shash, int _norep=0);

	virtual     ~bloomfilter();

	void	    clear();

	bool 	    insert(unsigned char *in, int len);
	bool 	    check(unsigned char *in, int len);

	bool        set_bits(const unsigned* bits, int len = 0);
	bool        check_bits(const unsigned* bits, int len = 0);

	unsigned    get_nzero() {return num_zeros;}

	void 	    print(FILE* fp = stderr);

    // bool copy(const char* buffer, size_t len);

    int serialize(char* buffer, size_t max_len);

    static bloomfilter* deserialize(const char* buffer, size_t len);

    };

#endif

