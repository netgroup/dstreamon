
#pragma once

#include <stdexcept>
#include "Digest.hpp"

#define s_bitmask(bits) ((1 << bits)-1)

class HashSplitter
{
    uint32_t* buffer;
    size_t length;

    mutable uint32_t ptr;

public:

    HashSplitter() : buffer(NULL), length(0), ptr(0)
    {}

    HashSplitter( const Digest& _dig, uint32_t beginFrom = 0 ) : buffer( (uint32_t*)(_dig.value() + beginFrom) ), 
                                                                 length( _dig.size() * 32 ),
                                                                 ptr(0)
    {}

    HashSplitter( const uint32_t* _buffer, size_t _length ) : buffer( (uint32_t*)_buffer ),
                                                              length(_length * 32),
                                                              ptr(0)
    {}


    void set( const Digest& dig, uint32_t beginFrom = 0 )
    {
        buffer = (uint32_t*) ( dig.value() + beginFrom );
        length = dig.size() * 32;
        ptr    = 0;
    }


    void set( const uint32_t* _buffer, size_t _length, uint32_t beginFrom = 0 )
    {
        buffer = (uint32_t*) (_buffer + beginFrom);
        length = _length * 32;
        ptr = 0;
    }

    void reset()
    {
        ptr = 0;
    }


    bool next(uint32_t bits, uint32_t* out) const
    {
        // extract bits from buffer

        if (ptr > length || out == NULL) return false;


        uint32_t s_blk = ptr / 32;
        uint32_t s_bit = ptr % 32;
        int32_t carry = s_bit + bits - 32;

        *out = (buffer[s_blk] >> s_bit) & s_bitmask(bits);

        if (carry > 0)
        {
            s_blk++;

            *out |= ((buffer[s_blk] & s_bitmask(carry)) << (bits-carry));
        }

        ptr += bits;

        return true;
    }

};
