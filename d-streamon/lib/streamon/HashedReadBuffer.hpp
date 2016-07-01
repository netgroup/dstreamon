
/* ###########################################
 *
 * HASHED READBUFFER
 *
 * A simple wrapper around common data types
 * with support with cached hash
 *
 * NOTE: as not owned vars can be deleted,
 *       the user must care about this when
 *	     access to raw bytes
 *
 * 
 * Author: Giulio Picierro, CNIT
 *
 * Mail: giulio.picierro@uniroma2.it
 * 
 * Version: 1.0
 *
 * ##########################################
 */

#pragma once

#include <string>

#include <stdint.h>

#include "IBuffer.hpp"
#include "Digest.hpp"
#include "ReadBuffer.hpp"

#include <hash/C/bob.h>

#define DIGEST_SIZE 7

class HashedReadBuffer : public ReadBuffer
{

    typedef ReadBuffer super;

protected:

    mutable uint32_t digest[DIGEST_SIZE];

    const Digest dig;


    void digest_reset() const
    {
        for (int i=0; i < DIGEST_SIZE; i++)
            digest[i] = i;
    }


public:

    HashedReadBuffer() : dig(digest, DIGEST_SIZE)
    {
       digest_reset(); 
    }

    template< typename T>
    HashedReadBuffer(const T& var) : super(var), dig(digest, DIGEST_SIZE)
    {
        digest_reset();
    }


    virtual Digest GetHash() const
    {
        if (digest[0] != true)
        {
            digest_reset();

            // compute hash;

            hashlittle2( Data, Count, &digest[1], &digest[2] );
            hashlittle2( Data, Count, &digest[3], &digest[4] );
            hashlittle2( Data, Count, &digest[5], &digest[6] );

            digest[0] = true;
        }

        return dig;
    }

};
