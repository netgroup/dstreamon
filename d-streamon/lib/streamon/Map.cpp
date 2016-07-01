
#include "IDictionary.hpp"
#include "ReadBuffer.hpp"

#include <cassert>
#include <map>

template<typename K, typename V>
class Map : public IDictionary<V>
{
    std::map<K, V> dict;

    public:


    virtual V* add(IReadBuffer& key, const V value)
    {
        assert( key.GetLength() == sizeof(K) ); 

        auto _key = *reinterpret_cast<const K*>( key.GetRawData() );

        dict[_key] = value;

        return &dict[_key];
    }

    
    virtual V* get(IReadBuffer& key) const
    {
        assert( key.GetLength() == sizeof(K) ); 

        auto _key = *reinterpret_cast<const K*>( key.GetRawData() );

        auto it = dict.find(_key);

        if (it != dict.end()) return (V*) &(it->second);

        return NULL;
    }


    virtual bool remove(IReadBuffer& key)
    {
        assert( key.GetLength() == sizeof(K) ); 

        auto _key = *reinterpret_cast<const K*>( key.GetRawData() );

        return dict.erase(_key);
    }


    virtual void clear()
    {
        dict.clear();
    }


    virtual ~Map()
    {
    }

};
