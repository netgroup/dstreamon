
#pragma once

#include "IDictionary.hpp"
#include "HashSplitter.hpp"

#include <cstring>
#include <iostream>

template<typename V>
class DLeft : public IDictionary<V>
{

public:

    DLeft( int _nhash, int _shash ) : nhash(_nhash), 
                                      shash(_shash),
                                      table_count(1)
    {
        Data = new dleft_node*[ _nhash ];

        Data[0] = new dleft_node[ 1 << _shash ]();
    }

    
    V* reserve(IReadBuffer& key)
    {
        Digest digest = key.GetHash(); // const uint32_t* _hash = digest.value();

        const uint32_t fhash = _fnv_modified_hash( key.GetRawData(), key.GetLength() );

        uint32_t place = 0;
        
        // get a placement on dleft

        int i = 1;

        splitter.set(digest, 1);

        do
        {
            for (; i <= table_count; i++)
            {
                bool good = splitter.next( shash, &place );         // get next hash

                if (!good) return NULL;

                dleft_node* node = &Data[i-1][place];

                if (node->HashKey == 0)
                {
                    /* found free block */

                    node->HashKey = fhash;

                    return &(node->Object);
                }
                else if ( node->HashKey == fhash )
                {
                    return &(node->Object);
                }

                // other cases means collision so we need to recycle
            }

            // place not found, we need to expand (if possible)
         }
         while (expand());

        return NULL;
    }


    virtual V* add(IReadBuffer& key, const V value)
    {
        V* slot = reserve(key);

        if (slot) *slot = value;

        return slot;
    }


    virtual V* get(IReadBuffer& key) const
    {
        Digest digest = key.GetHash();

        const uint32_t fhash = _fnv_modified_hash(key.GetRawData(), key.GetLength());

        uint32_t place = 0;

        splitter.set(digest, 1);

        // search

        for (int i=1; i <= table_count; i++)
        {
            bool good = splitter.next(shash, &place);

            if (!good) return NULL;

            dleft_node* node = &Data[i-1][place];

            if (node->HashKey == fhash)
            {
                // found

                return &(node->Object);
            }
        }

        return NULL;
    }



    virtual bool remove(IReadBuffer& key)
    {
        Digest digest = key.GetHash();
        const uint32_t fhash = _fnv_modified_hash( key.GetRawData(), key.GetLength() );

        uint32_t place = 0;

        splitter.set(digest, 1);


        /* search and destroy */

        for (int i=1; i <= table_count; i++)
        {
            bool good = splitter.next(shash, &place);

            if (!good) return false;

            dleft_node* node = &Data[i-1][place];

            if (node->HashKey == fhash)
            {
	            /* destroy */

	            node->HashKey = 0;

	            return true;
            }
        }

        return false;
    }


    virtual void clear()
    {
        uint32_t table_size_bytes = (1 << shash)*sizeof(dleft_node);

        for (int i=0; i < table_count; i++)
        {
            memset(Data[i], 0, table_size_bytes );
        }
    }


    void remove( V* element )
    {
        assert(element);

        auto node = (dleft_node*) element;

        /* for each table */

        uint32_t table_len = (1 << shash);


        for (int i=0; i < table_count; i++)
        {            
            if (node >= Data[i] && node < Data[i] + table_len )
            {
                node->HashKey = 0;
                return;
            }
        }

        throw std::runtime_error("element not owned by this dleft hash");
    }


    virtual ~DLeft()
    {
        // std::cout << "destructor called\n";

        for (int i=0; i < table_count; i++)
            delete[] Data[i];

        delete[] Data;
    }

private:

    inline uint _fnv_modified_hash(const unsigned char* buffer, int len) const
    {
        const uint p = 16777619U;
        uint hash = 2166136261U;

        for (int i=0; i < len; i++)
          hash = (hash ^ buffer[i]) * p;

        hash += hash << 13;
        hash ^= hash >> 7;
        hash += hash << 3;
        hash ^= hash >> 17;
        hash += hash << 5;
        return hash;
    }


    bool expand()
    {
        if (table_count >= nhash) return false;

        Data[table_count] = new dleft_node[ 1 << shash ]();

        table_count++;

        return true;
    }


    struct dleft_node
    {
        V Object;
        uint32_t HashKey;

        dleft_node() : HashKey(0) { }
    };


// CLASS ATTRIBUTES

    dleft_node** Data;

    int nhash;
    int shash;
    int table_count;

    mutable HashSplitter splitter;


};
