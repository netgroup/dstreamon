/*
 * @file    bob.hpp
 * @author  Fabrizio Nuccilli
 * @version 1.0
 *
 * @section LICENSE
 *
 *
 * @section DESCRIPTION
 *
 * The class represents the BOB function
 *
 */


#ifndef BOB_HPP
#define	BOB_HPP
#define dgstlenbob 4
#define INITNUMB 0xf1ea5eed //ratio init number

#include"hash.hpp"


namespace bm {

class BOB : public HASH
{
    

public:

    BOB():HASH(dgstlenbob){}

    ~BOB(){}

    /*
     * this method compute the digest of a key
     * @param key is the key that is to be hashed
     * @param len is the key length
     * @param output is the array where the message digest is stored
     */
    virtual void compute(const unsigned char key[], int len, unsigned char output[]);

    /*
     * this method hashes a file
     * @param filepath the file path
     * @param output is the array where the message digest is stored
     */
    virtual void computeFile(const char *filepath, unsigned char output[]);

    /*
     * this method implements a simple way to hash a key and a message
     * output = H(KEY||MESSAGE) with || concatenation
     * Attention: this is not implemented as HMAC because BOB
     * is not a cryptographic hash function
     * @param key is the key
     * @param keylen is the key length
     * @param input is the original message
     * @param ilen is the length of the original message
     * @param output is the array where the message digest is stored
     */
    virtual void compute(const unsigned char *key, int keylen, const unsigned char *input, int ilen,
                unsigned char output[]);

};

}


#endif	/* BOB_HPP */

