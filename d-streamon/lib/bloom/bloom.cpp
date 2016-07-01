//#include "sha1.h"
#include "khash.h"
#include "bloom.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>

using namespace std;

//
// CONSTRUCTOR
//
bloomfilter::bloomfilter(int _nhash, int _shash, int _norep) :
    khash(_nhash, _shash, _norep)
{
    assert((shash>=1)&&(shash<=31)); 

	// parameters settings
    mmax     = 0x00000001 << shash;
    mmax_arr = (shash<3) ? 1 : 0x00000001 << (shash-3);
    
	// bit array allocation
    m = new unsigned char[mmax_arr];
    clear();
}


//
// DESTRUCTOR
//
bloomfilter::~bloomfilter() {
    delete [] m;
    }


//
// CLEAR
//
void bloomfilter::clear() {
    for(unsigned i=0; i<mmax_arr; i++) m[i]=0;
    num_zeros=mmax;
    }


//
// MAIN FUNCTIONS:
//	- INSERT
//	- CHECK
//
bool bloomfilter::insert(unsigned char *in, int len) {
    compute(in,len);
    int extra_ones = 0;
    for(int i=1; i<=nhash; i++) {
	unsigned b = H(i) >> 3;
	assert(b<mmax_arr);
	unsigned char t = (unsigned char)(H(i) & bitmask(3));
	assert(t<8);
	unsigned char bit = 0x01 << t;
	if ((m[b] & bit) == 0) {
	    extra_ones++;
	    m[b] = m[b] | bit;
	    }
	}
    if (extra_ones==0) return 0;
    num_zeros-=extra_ones;
    return 1;
    }


bool bloomfilter::check(unsigned char *in, int len) {
    compute(in,len);
    for(int i=1; i<=nhash; i++) {
	unsigned b = H(i) >> 3;
	assert(b<mmax_arr);
	unsigned char t = (unsigned char)(H(i) & bitmask(3));
	assert(t<8);
	unsigned char bit = 0x01 << t;
	if ((m[b] & bit) == 0) return 0;
	}
    return 1;
    }


bool bloomfilter::set_bits(const unsigned* bits, int len)
{

  // int nbits = (len > 0 ? len : nhash);

  split(bits, len);

  int extra_ones = 0;
  for(int i=1; i<=nhash; i++) {
    unsigned b = H(i) >> 3;
    assert(b<mmax_arr);
    unsigned char t = (unsigned char)(H(i) & bitmask(3));
    assert(t<8);
    unsigned char bit = 0x01 << t;
    if ((m[b] & bit) == 0) {
      extra_ones++;
      m[b] = m[b] | bit;
    }
  }

  if (extra_ones==0) return 0;
  num_zeros-=extra_ones;
  return 1;
}


bool bloomfilter::check_bits(const unsigned* bits, int len)
{
  // int nbits = (len > 0 ? len : nhash);

  split(bits, len);
  
  for(int i=1; i<=nhash; i++) {
    unsigned b = H(i) >> 3;
    assert(b<mmax_arr);
    unsigned char t = (unsigned char)(H(i) & bitmask(3));
    assert(t<8);
    unsigned char bit = 0x01 << t;
    if ((m[b] & bit) == 0) return 0;
  }
  return 1;
}


/*
void bloomfilter::print(FILE* fp) {
    fprintf(fp,"BF status: %d zeros;",num_zeros);
    for(unsigned i=0; i<mmax_arr; i++) {
	if (i%8 == 0) fprintf(fp,"\n");
	bitprint(m[i], fp);
	}
    fprintf(fp,"\n");
    }
*/

int bloomfilter::serialize(char* buffer, size_t max_len)
{
    assert(buffer);

    size_t buffer_size = mmax_arr + sizeof(nhash) + sizeof(shash);

    if (max_len < buffer_size)
    {
        return -1;
    }

    char* ptr = buffer;

    memcpy(ptr, &nhash, sizeof(nhash));

    ptr += sizeof(nhash);

    memcpy(ptr, &shash, sizeof(shash));

    ptr += sizeof(shash);

    memcpy(ptr, m, mmax_arr);

    return buffer_size;
}


bloomfilter* bloomfilter::deserialize(const char* buffer, size_t len)
{
    assert(buffer);

    if (len < 8) return NULL;

    const char* ptr = buffer;

    int nhash = *reinterpret_cast<const int*>(ptr);
    int shash = *reinterpret_cast<const int*>(ptr+sizeof(int));

    ptr += 8;
    len -= 8;

    size_t memory_size = (1 << shash)/8;

    if (len < memory_size) return NULL;

    bloomfilter* bf = new bloomfilter(nhash, shash, 1);

    memcpy(bf->m, ptr, memory_size);

    return bf;
}
