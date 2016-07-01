#include "sha1.h"
#include "khash.h"
#include "hash/C/bob.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>

#include "MurmurHash3.h"

#include "spooky.h"

#define HASH_INTEGERS 6
#define HASH_BITS     192
#define HASH_BYTES    24

using namespace std;

//
// CONSTRUCTOR
//
khash::khash(int _nhash, int _shash, int _norep) {

	// obvious asserts
    assert(_nhash>=1);
    assert(_shash>=1);
    assert((_norep==0)||(_norep==1));


	// for simplicity, using unsigned as maximum digest size
	// consequence: no more than 31 bits
    assert(_shash<32);

	// assign constants
    nhash = _nhash;
    shash = _shash;
    norep = _norep;
    digest = 0x00000001 << shash;

	// assert that infinite cycle because of insufficient number 
	// of hash will not occur
   assert((norep==1)?(unsigned(nhash) <= (unsigned(1) << shash)):1);

	// allocate output vector - result in [1:nhash]
    res   = new unsigned[nhash+1];
    }




//
// DESTRUCTOR
//
khash::~khash() {
    delete [] res;
    }



void khash::split(const unsigned* msgdig, int len)
{
  // now cycle on digest to extract khash bits from hash

    int ptr = 0;		
    int k=1;
    while (k<=nhash)
    {
      int s_blk = ptr / 32;
      int s_bit = ptr % 32;
      res[k] = (msgdig[s_blk] >> s_bit) & bitmask(shash);
      int carry = s_bit + shash - 32;

      /*
      // compute next sha if needed. Note that this is done 
      // also if carry=0 for subtle reason a bit long to explain
      if ((carry>=0)&&(s_blk==HASH_INTEGERS-1))
      {	    
	hashlittle2(in, len, &msgdig[0], &msgdig[1]);
	hashlittle2(in, len, &msgdig[2], &msgdig[3]);
	hashlittle2(in, len, &msgdig[4], &msgdig[5]);

	copy_sha(msgdig,md);
	ptr -= HASH_BITS; 	// trick to make it fit with what follows
	s_blk=-1;  		// trick to make it fit with what follows
      }
      */

      // now conclude computation for res, in the case 
      // computation spans over two digest unsigned blocks
      if (carry>0)
      {
	s_blk++;
	res[k]=res[k]|((msgdig[s_blk] & bitmask(carry)) << (shash-carry));
      }

      // more forward in the digest domain
      ptr += shash;
      assert((ptr>=0)&&(ptr<=HASH_BITS));

      // finally, if norep flag is set, move to the next res 
      // only if this computed value is NOT repeated;
      // otherwise start next iteration with the same 
      // value k (indeed k-- cancels the next k++)
      if (norep)
	for(int i=1; i<k; i++) if (res[i]==res[k]) k--;
      k++;
    }

    /*
    for (int i=1; i <= nhash; i++)
      printf("%u ", res[i]);

    printf("\n");
    */
}



//
// MAIN FUNCTION: COMPUTE K DIGEST
//
void khash::compute(unsigned char *in, int len, unsigned* copy_buffer) {
    assert(len>0);
    unsigned    msgdig[HASH_INTEGERS] = {1,2,3,4,5,6};

	// compute first SHA hash; should be all needed most of the time;
	// more SHA are always needed when shash*nhash>160; more SHA MAY be 
	// needed when no repetition is required... in this case this 
	// depends on the input
    /*
    sha.Reset();
    sha.Input(in, len);
    bool outcome = sha.Result(msgdig);
    assert(outcome);
*/

     /* MurmurHash3_x86_128 ( in, len,
			  1, msgdig );
    */

    /*
    msgdig[0] = SpookyHash::Hash32(in, len, 1);
    msgdig[1] = SpookyHash::Hash32(in, len, 2);
    msgdig[2] = SpookyHash::Hash32(in, len, 3);
    msgdig[3] = SpookyHash::Hash32(in, len, 4);
    */
	/*
	msgdig[0] = hashlittle(in, len, 1);
	msgdig[1] = hashlittle(in, len, 2);
	msgdig[2] = hashlittle(in, len, 3);
	msgdig[3] = hashlittle(in, len, 4);
	msgdig[4] = hashlittle(in, len, 5);
	*/	

	hashlittle2(in, len, &msgdig[0], &msgdig[1]);
	hashlittle2(in, len, &msgdig[2], &msgdig[3]);
	hashlittle2(in, len, &msgdig[4], &msgdig[5]);

	//SpookyHash::Hash128(in, len, (uint64_t*)&(msgdig[0]), (uint64_t*)&(msgdig[2]));

	// copy sha message digest into cache; frequently not needed 
	// and can be optimized out by checking whether this copy 
	// will be needed or not;
    unsigned char md[HASH_BYTES];
	copy_sha(msgdig,md);

#ifdef _KHASH_DEBUG 
    fprintf(stderr, "\nDEBUG PRINT - original SHA1\n");
    for(int b=0; b<5; b++) {
	bitprint(msgdig[b], stderr);
    	fprintf(stderr,"\n");
	}
    fprintf(stderr, "            - and its cache copy\n");
    for(int b=1; b<=20; b++) {
	bitprint(md[b-1], stderr);
    	if (b%4==0) fprintf(stderr,"\n");
	}
#endif

	// now cycle on digest to extract khash bits from SHA digest
    int ptr = 0;		
    int k=1;
    while (k<=nhash) {
	int s_blk = ptr / 32;
	int s_bit = ptr % 32;
	res[k] = (msgdig[s_blk] >> s_bit) & bitmask(shash);
	int carry = s_bit + shash - 32;

#ifdef _KHASH_DEBUG 
fprintf(stderr, "DDD k=%d: blk=%d bit=%d carry=%d ",k,s_blk,s_bit,carry);
bitprint(res[k],shash,stderr);
fprintf(stderr, "\n");
#endif

		// compute next sha if needed. Note that this is done 
		// also if carry=0 for subtle reason a bit long to explain
	if ((carry>=0)&&(s_blk==HASH_INTEGERS-1)) {
	    
	   /*
    	    sha.Reset();
    	    sha.Input(md, HASH_BYTES);
    	    bool outcome = sha.Result(msgdig);
    	    assert(outcome);
		*/
	    /*MurmurHash3_x86_128 ( md, HASH_BYTES,
	      1, msgdig );
*/
	    /*
	    msgdig[0] = SpookyHash::Hash32(in, len, 1);
	    msgdig[1] = SpookyHash::Hash32(in, len, 2);
	    msgdig[2] = SpookyHash::Hash32(in, len, 3);
	    msgdig[3] = SpookyHash::Hash32(in, len, 4);
	    */
		/*
		msgdig[0] = hashlittle(in, len, 1);
		msgdig[1] = hashlittle(in, len, 2);
		msgdig[2] = hashlittle(in, len, 3);
		msgdig[3] = hashlittle(in, len, 4);
		msgdig[4] = hashlittle(in, len, 5);
		*/

		hashlittle2(in, len, &msgdig[0], &msgdig[1]);
		hashlittle2(in, len, &msgdig[2], &msgdig[3]);
		hashlittle2(in, len, &msgdig[4], &msgdig[5]);


	//SpookyHash::Hash128(in, len, (uint64_t*)&(msgdig[0]), (uint64_t*)&(msgdig[2]));

	    copy_sha(msgdig,md);
	    ptr -= HASH_BITS; 	// trick to make it fit with what follows
	    s_blk=-1;  		// trick to make it fit with what follows
	    }

		// now conclude computation for res, in the case 
		// computation spans over two digest unsigned blocks
	if (carry>0) {
	    s_blk++;
	    res[k]=res[k]|((msgdig[s_blk] & bitmask(carry)) << (shash-carry));
	    }

		// more forward in the SHA digest domain
	ptr += shash;
	assert((ptr>=0)&&(ptr<=HASH_BITS));

		// finally, if norep flag is set, move to the next res 
		// only if this computed value is NOT repeated;
		// otherwise start next iteration with the same 
		// value k (indeed k-- cancels the next k++)
	if (norep)
	    for(int i=1; i<k; i++) if (res[i]==res[k]) k--;
	k++;
	}
		
		
#ifdef _KHASH_DEBUG 
    fprintf(stderr,"\nDEBUG PRINT - resulting short digests\n");
    for(int k=1; k<=nhash; k++) {
	bitprint(res[k],shash,stderr);
	fprintf(stderr,"\n");
	}
#endif

    if (copy_buffer != NULL)
    {
      assert( nhash <= 8 );

      memcpy( &copy_buffer[1], &res[1], nhash*sizeof(unsigned) );
    }

}



//
// private function; it receives as input a message digest organized into 
// a vector of 5 unsigned , and copies it into a vector of 20 unsigned char
//
void khash::copy_sha(unsigned* vin, unsigned char* vout) {
	int blk_in=0;
	int blk_out=0;
	unsigned char c;
	while(blk_in<HASH_INTEGERS) {
	    c = (unsigned char)((vin[blk_in] >> (blk_out%4)*8) & 0x000000FF);
	    vout[blk_out] = c;
	    blk_out++;
	    if (blk_out%4 == 0) blk_in++;
	    }
	assert(blk_in==HASH_INTEGERS);
	assert(blk_out == HASH_BYTES);
	}



void khash::bitprint(unsigned t, int upto, FILE* fp) {
	assert((upto>=1) && (upto<=32));
	for(int n=0; n<upto; n++) {
	    if ((n>0)&&(n%8==0)) fprintf(fp," ");
	    if (t&0x00000001) fprintf(fp,"1"); else fprintf(fp,"0");
	    t = t >> 1;
	    }
	fprintf(stderr," ");
	}

void khash::bitprint(unsigned char t, FILE* fp) {
	for(int n=0; n<8; n++) {
	    if (t&0x00000001) fprintf(fp,"1"); else fprintf(fp,"0");
	    t = t >> 1;
	    }
	fprintf(stderr," ");
	}

