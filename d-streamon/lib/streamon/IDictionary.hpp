
#pragma once

#include "IBuffer.hpp"

template<typename V>
class IDictionary
{

public:

    virtual V* add(IReadBuffer& key, const V value) = 0;

    virtual V* get(IReadBuffer& key) const = 0;

    virtual bool remove(IReadBuffer& key) = 0;

    virtual void clear() = 0;

    virtual ~IDictionary()
    {
    }
 

};
