
#ifdef _S_DLEFT_OLD

#include "khash.h"

#include <stdexcept>

#include "../lib_botstream/IBuffer.hpp"


template<typename V>
class DLeft : public khash
{
  struct dleft_node
  {
    V Object;
    uint32_t Hash;
  };


  dleft_node* Data;

  uint32_t offset;


  inline uint _fnv_modified_hash(const unsigned char* buffer, int len)
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

public:

  DLeft( int _nhash, int _shash ) : khash(_nhash, _shash, 1)
  {
    offset = (1 << _shash);

    Data = new dleft_node[ _nhash * offset ];

    /* replace with malloc (to realloc in case of)

       Data = calloc( _nhash/2, sizeof(dleft_node) );
    */
  }


  V* add( const unsigned char* buffer, int len, V object, const uint32_t hash[] = NULL)
  {
    if (hash == NULL)
      compute( (unsigned char*)buffer, len);
    else
      split(hash);

    uint32_t fhash = _fnv_modified_hash( buffer, len );


    /* get a placement on dleft */

    for (int i=1; i <= nhash; i++)
    {

      dleft_node* node =  &Data[ (i-1)*offset + H(i) ];


      if (node->Hash == 0)
      {
	/* found free block */

	node->Hash = fhash;
	node->Object = object;

	return &(node->Object);
      }
      else if ( node->Hash == fhash )
      {
	node->Object = object;

	return &(node->Object);
      }

      /* other cases means collision so we need to recycle */

      /* assuring that there is enough space */

      // if ( allocated < nhash ):
      // 
      //    factor = 2 * allocated;
      // 
      //    Data = realloc(factor);
      //    
      //    allocated = nhash;
    }

    return NULL;
  }


  V* add( const IReadBuffer& key, V value, const uint32_t hash[] = NULL)
  {

    V* slot = reserve(key, hash);

    if (slot) *slot = value;

    return slot;
    
    /*
    return add( key.GetRawData(),
		key.GetLength(),
		value,
		hash );
    */

  }


  V* reserve( const IReadBuffer& key, const uint32_t hash[] = NULL )
  {
    const unsigned char* buffer = key.GetRawData();
    size_t len = key.GetLength();


    if (hash == NULL)
      compute( (unsigned char*)buffer, len);
    else
      split(hash);

    uint32_t fhash = _fnv_modified_hash( buffer, len );


    /* get a placement on dleft */

    for (int i=1; i <= nhash; i++)
    {

      dleft_node* node =  &Data[ (i-1)*offset + H(i) ];


      if (node->Hash == 0)
      {
	/* found free block */

	node->Hash = fhash;

	return &(node->Object);
      }
      else if ( node->Hash == fhash )
      {
	return &(node->Object);
      }

      /* other cases means collision so we need to recycle */

      /* assuring that there is enough space */

      // if ( allocated < nhash ):
      // 
      //    factor = 2 * allocated;
      // 
      //    Data = realloc(factor);
      //    
      //    allocated = nhash;
    }

    return NULL;
  }


  V* get( const unsigned char* buffer, int len, const uint32_t hash[] = NULL)
  {
    if (hash == NULL)
      compute( (unsigned char*) buffer, len);
    else
      split(hash, len);

    uint32_t fhash = _fnv_modified_hash( buffer, len );

    /* search */

    for (int i=1; i <= nhash; i++)
    {

      dleft_node* node =  &Data[ (i-1)*offset + H(i) ];


      if (node->Hash == fhash)
      {
	/* found it */

	return &node->Object;
      }
    }

    return NULL;
  }


  V* get( const IReadBuffer& key, const uint32_t hash[] = NULL)
  {
    return get( key.GetRawData(),
		key.GetLength(),
		hash );
  }


  
  bool contains( const IReadBuffer& key, const uint32_t hash[] = NULL)
  {
    return contains( key.GetRawData(),
		     key.GetLength(),
		     hash );
  }


  bool contains( const unsigned char* buffer, int len, const uint32_t hash[] = NULL)
  {
    return get(buffer, len, hash) != NULL;
  }


  void remove( const unsigned char* buffer, int len, const uint32_t hash[] = NULL )
  {
    if ( hash == NULL)
      compute( (unsigned char*) buffer, len );
    else
      split(hash, len);

    uint32_t fhash = _fnv_modified_hash( buffer, len );


    /* search and destroy */

    for (int i=1; i <= nhash; i++)
    {
      dleft_node* node =  &Data[ (i-1)*offset + H(i) ];

      if (node->Hash == fhash)
      {
	/* destroy */

	node->Hash = 0;

	return;
      }
    }       
  }


  void remove( const IReadBuffer& key, const uint32_t hash[] = NULL )
  {
    remove( key.GetRawData(),
	    key.GetLength(),
	    hash );
  }


  void remove( V* element )
  {
    assert(element);

    auto node = (dleft_node*) element;

    if (node >= Data && node < Data + nhash*offset )
    {
      node->Hash = 0;
    }
    else throw std::runtime_error("element not owned by this dleft hash");
  }


  virtual ~DLeft()
  {
    delete[] Data;
  }
  

};

#endif
