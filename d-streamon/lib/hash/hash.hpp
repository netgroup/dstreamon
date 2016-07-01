/**
 * @file    hash.hpp
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
 * The class represents an abstract hash function
 * 
 */


#ifndef HASH_HPP
#define	HASH_HPP

namespace bm {

class HASH {

public:

    /*
     * this constructor can not be called because HASH is an abstract class
     * but it is used by sub-classes
     * @param m is the message digest lenght of the hash
     */
    HASH(unsigned int m):msgdigestlen(m){}
    virtual ~HASH(){};
    
    /*
     * a simple get method
     * @return the digest length
     */
    unsigned int getDigestLength(){return msgdigestlen;}
    
    /*
     * this method compute the digest of a key
     * @param key is the key that is to be hashed
     * @param len is the key length
     * @param output is the array where the message digest is stored 
     */
    virtual void compute(const unsigned char key[], int len, unsigned char output[])=0;
    
    /*
     * this method hashes a file
     * @param filepath the file path
     * @param output is the array where the message digest is stored 
     */
    virtual void computeFile(const char *filepath, unsigned char output[])=0;
    
    /*
     * HMAC
     * this method implements a message authentication code (MAC) 
     * involving an hash function in combination with a secret key
     * @param key is the key 
     * @param keylen is the key length
     * @param input is the original message
     * @param ilen is the length of the original message
     * @param output is the array where the message digest is stored 
     */
    virtual void compute(const unsigned char *key, int keylen, const unsigned char *input, int ilen,
                unsigned char output[])=0;

private:
    unsigned int msgdigestlen;

};

}
#endif	/* HASH_HPP */

