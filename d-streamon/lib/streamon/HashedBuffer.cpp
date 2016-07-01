
/* ###########################################
 *
 * HASHED BUFFER
 *
 * A simple template class that allows handling 
 * of data types in an uniform way
 * (as a sequence of bytes) and also
 * allows concatenation of sequences of bytes.
 * 
 * It also has a cached hash.
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


#include <string>

#include <stdint.h>

#include "Buffer.cpp"
#include "Digest.hpp"

#include "../hash/C/bob.h"


template< size_t S, int D=7 >
class HashedBuffer : public Buffer<S>
{

    typedef Buffer<S> super;

protected:

    mutable uint32_t digest[D];

    const Digest dig;


    void digest_reset() const
    {
        for (int i=0; i < D; i++)
            digest[i] = i; 
    }

public:

    HashedBuffer() : dig(digest, D)
    {
        digest[0] = false;
    }

    HashedBuffer(const HashedBuffer& other) : dig(digest, D)
    {
        append(other.Data, other.Count, other.Type);
    }

    HashedBuffer& operator=(const HashedBuffer& other)
    {
        this->Reset();

        append(other.Data, other.Count, other.Type);

        return *this;
    }

    template<typename T>
    Buffer<S>& append(T Val, buffer_type Ty = buffer_type::UNKNOWN)
    {
        return append( &Val, sizeof(Val), Ty );
    }


    virtual Buffer<S>& append(const void* Ptr, int Len, buffer_type Ty = buffer_type::UNKNOWN)
    {
        digest[0] = false;  // reset hashed flag

        return super::append(Ptr, Len, Ty);
    }


    void Reset()
    {
        digest[0] = false;

        super::Reset();
    }


    virtual Digest GetHash() const
    {
        if (digest[0] != true)
        {
            // compute hash;

            digest_reset();

            hashlittle2( super::Data, super::Count, &digest[1], &digest[2] );
            hashlittle2( super::Data, super::Count, &digest[3], &digest[4] );
            hashlittle2( super::Data, super::Count, &digest[5], &digest[6] );

            digest[0] = true;
        }

        return dig;
    }

};
