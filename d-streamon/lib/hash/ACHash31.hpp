/**
 * @file    ACHash31.hpp
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
 * This class represents a simple hash: h(i)=(a*i+b)mod p
 * It is optimised for a p = 2^31-1
 * and it works with unsigned int
 */




#ifndef ACHASH31_HPP
#define	ACHASH31_HPP

#define dgstlenhsm 4
#define mod 0x2FFFFFFF

#include "hash.hpp"

namespace bm {

class ACHash31 : public HASH
{


public:

    ACHash31(unsigned int a, unsigned int b):
        HASH(dgstlenhsm),SMOD_A(a),SMOD_B(b),SMOD_MOD(mod){}


     /*
     * this method compute the digest of a key
     * @param key is the key that is to be hashed
     * @return the message digest
     */
    unsigned int compute(unsigned int key);

    /*
     * this method compute the digest of a key
     * @param key is the key that is to be hashed
     * @param a is the multiplied int
     * @param b is the added int
     * @param mod is the module
     * @return is the message digest
     */
    unsigned int compute(unsigned int key, unsigned int a, unsigned int b);

    /*
     * this method compute the digest of a key
     * if the key is more then 4 bytes, this method split it in len / 4 pieces
     * then all pieces are added to create the new key which is hashed
     * @param key is the key that is to be hashed
     * @param len is the key length
     * @param output is the array where the message digest is stored
     */
    virtual void compute(const unsigned char key[], int len, unsigned char output[]);

    /*
     * this method hashes a file which contains integer
     * it adds all the integer to create a key and then computes the hash function
     * @param filepath the file path
     * @param output is the array where the message digest is stored
     */
    virtual void computeFile(const char *filepath, unsigned char output[]);

    /*
     * HMAC
     * this method implements a message authentication code (MAC)
     * involving this hash in combination with a secret key
     * also in this case both the key and the msg are splitted in pieces which max
     * length is 32 bit and then they are added and hashed
     * @param key is the key
     * @param keylen is the key length
     * @param input is the original message
     * @param ilen is the length of the original message
     * @param output is the array where the message digest is stored
     */
    virtual void compute(const unsigned char *key, int keylen, const unsigned char *input, int ilen,
                unsigned char output[]);

private:
    unsigned int SMOD_A;
    unsigned int SMOD_B;
    unsigned int SMOD_MOD;
};

}



#endif	/* HASH_SMOD_HPP */

