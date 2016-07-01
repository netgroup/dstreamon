/**
 * @file    khash.hpp
 * @author  Fabrizio Nuccilli 
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * The class represents an object that can make several hash functions
 * at one time
 */
 


#ifndef KHASH_HPP
#define	KHASH_HPP

#include "hash.hpp"
#include <cassert>
#include <vector>
#include <cstring>



namespace bm {


class KHASH {
    
    public:

        /*
         *this constructor create a KHASH object which can compute n hash
         * with a prefixed digest using a hash function (sha1, sha256, etc...)
         * @param _hash pointer to the hash used
         * @parma _nhash number of parallel hash
         * @param _shash digest of each hash
         * @param _norep if true each message digest is different from other
         *
         * if the length of the message digest of _hash is greater than _nhash*_shash
         * the message digest itself is rehashed
         */
	KHASH(HASH* _hash, unsigned int _nhash, unsigned int _shash, bool _norep=false);

        /*
         *copy constructor
         */
        KHASH(const KHASH &);

        /*
         * assignment operator  
         */
        KHASH & operator = (const KHASH &);

        /*
         * destructor
         */
	~KHASH();

        /*
         *this is the main function: it computes the hash function and divide
         * the message digest into its sub-digest
         * @param in pointer to the key to be hashed
         * @param len key length
         */
	void compute(unsigned char *in, int len);

        /*
         *this method provide access to the k [0-_nhash) sub-digest
         * @param k is the sub-digest index
         * @return  the k-th sub-digest
         */
	unsigned int H(unsigned int k) const
	{
	    assert((k<nhash));
	    return res[k];
	}
        
        /*
         * this method increase by 1 the number of hash computed
         * it has to be followed by a new call to compute 
         * the first nhash - 1 will be the same as before increase
         */
        void inc_nhash()
        {
            delete [] res;
            nhash++;
            res = new unsigned[nhash];
        }
        
        /*
         * this method decrease by 1 the number of hash computed
         * it has to be followed by a new call to compute 
         * the new nhash will be the same as before increase
         */
        void dec_nhash()
        {
            delete [] res;
            delete [] msgdig;
            nhash--;
            unsigned int bitneeded=((shash*nhash)<<3);
            hashtodo=bitneeded/(hashdiglen<<3);
            if (hashtodo*bitneeded!=(hashdiglen<<3))
                hashtodo++;
            msgdig=new unsigned char [hashtodo*hashdiglen+4];
            memset((void*)msgdig,0,hashtodo*hashdiglen+4);
            res = new unsigned[nhash];
        }

    private: 	

	HASH *hash;			//used hash function

        unsigned int hashdiglen;
    
	unsigned int nhash;		// number of hash functions

	unsigned int shash;		// size (bit) of each hash digest
    
	bool norep;			// flag: if true, repeat digest NOT possible

        unsigned int* res;		// output vector of digests [nhash]

	unsigned int hashtodo;

        unsigned char *msgdig;          //hash message digest

        



    };

}

#endif	/* KHASH_HPP */

