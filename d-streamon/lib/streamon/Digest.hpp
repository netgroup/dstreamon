
#pragma once

#include <cstdlib>
#include <stdint.h>
#include <cassert>

#define DETAG(data) ( reinterpret_cast<uint32_t*>( (data).intptr &~ 1UL) )
#define TAG(data) ( (data).intptr |= 1UL )

#define TAG_IS_SET(data) ( (data).intptr & 1UL )


class Digest
{

protected:

    union
    {
        uint32_t* array;
        unsigned long intptr;

    } data;

    size_t length;

public:

    Digest(size_t _size) : length(_size)
    {
        data.array = new uint32_t[_size];

        TAG(data);
    }

    Digest(uint32_t* _data, size_t _size) : length(_size)
    {
        assert(_data && _size > 0);

        data.array = _data;
    }


    uint32_t operator[](int i) const { return DETAG(data)[i]; }

    const uint32_t* value() const { return DETAG(data); }

    size_t size() const { return length; }

    void set(int i, uint32_t value) { DETAG(data)[i] = value; }

    virtual ~Digest()
    { 
        if ( TAG_IS_SET(data) )
        {
            delete[] DETAG(data);
        }
    }

};

