#ifndef _KHASH_H
#define _KHASH_H
// #include "sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>

// #include "MurmurHash3.h"
// #include "city.h"
// #include "spooky.h"

using namespace std;

/*****************************************************************************

			C L A S S   K H A S H

 specifies a generic multiple hash computator, using SHA1 as underlying hash
 function.

 khash uses, and exports to derived classes, the following parameters:
	
	int		nhash: 	number of hash functions, 1+, integer

	int		shash: 	size in bit of each hash digest, 1-31

	unsigned int 	digest:	size of each digest, being digest = 2^shash

 *****************************************************************************/
 

class khash {

    protected:
    	int 		nhash;		// number of hash functions
    	int 		shash;		// size (bit) of each hash digest
    	unsigned int 	digest;		// size of the digest: 1..digest

    private:
    	unsigned*	res;		// output vector of digests [1:nhash]
    	int		norep;		// flag: 1= repeat digest NOT possible
	// SHA1		sha;

    public:	// public methods

		    khash(int _nhash, int _shash, int _norep = 0);

		    ~khash();

	void	    compute(unsigned char *in, int len, unsigned* copy_buffer=NULL);
	void	    compute(string s);

	void        split(const unsigned* msgdig, int len=0);

	/*
	unsigned*   get_result() const
	{
	  return res;
	}
	*/

	unsigned    H(int k) {
			assert((k>=1)&&(k<=nhash)); 
			return res[k]; 
			};

	unsigned int maxdigest() {return digest;}

    

    protected: 	// protected methods

	unsigned    bitmask(int bit) {
    			assert((bit>=0)&&(bit<=32));
			if (bit==0) return 0x00000000;
			else if (bit==32) return 0xFFFFFFFF;
			else return ((unsigned(1) << bit) - 1) & 0xFFFFFFFF;
    			};


	void	    bitprint(unsigned char t, FILE* fp = stderr);

    private: 	// private methods

	void	    copy_sha(unsigned *vin, unsigned char* vous);

	void	    bitprint(unsigned t, int upto, FILE* fp = stderr);

	void	    bitprint(unsigned t, FILE* fp = stderr) {
				bitprint(t, 32,fp);
			   	}

    };

#endif
