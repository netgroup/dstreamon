#include "khash.h"
#include "tewma.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
using namespace std;


/************************************************************************
	CONSTRUCTOR, DESTRUCTOR, ELEMENTARY FUNCTIONS
 ************************************************************************/
TEWMA::TEWMA(int _nhash, int _shash, double _beta, double _w) 
: khash(_nhash, _shash, 1) {
    c = new double[maxdigest()];
    t = new uint64_t[maxdigest()];
    assert(_w>0);
    W=_w;
    set_beta(_beta);
    clear(0);
    }

TEWMA::~TEWMA() {
    delete [] c;
    delete [] t;
    }

void TEWMA::clear(uint64_t timestamp_start) {
    for(uint i=0; i<maxdigest(); i++) {
	c[i]=0;
	t[i]=timestamp_start;
	}
    }

void TEWMA::set_beta(double b) {
    assert((b>0)&&(b<1));
    beta=b;
    logbeta = -log(beta);
    }


/************************************************************************
	CORE FUNCTIONS
 ************************************************************************/

double TEWMA::add(unsigned char *in, int len, double quantity, uint64_t timestamp) {

	// first compute the hash functions via khash
    compute(in,len);

	// computes the new counter decayed value
    double minctr=c[H(1)];

    int64_t deltat;

    for(int i=1; i<=nhash; i++) {

        deltat = timestamp-t[H(i)];
        assert(deltat >= 0);

        // deltat = difftime(timestamp, t[H(i)])/W;
        t[H(i)] = timestamp;
        
        c[H(i)] *= pow(beta, double(deltat)/W);
        if (c[H(i)]<minctr) minctr=c[H(i)];
    }

	// now compute the new value including the insertion and update 
	// the filter by waterfilling the new counter array bins
    double newctr=minctr+quantity*logbeta;
    assert(newctr<=1.0e99);
    for(int i=1; i<=nhash; i++) if (c[H(i)]<newctr) c[H(i)]=newctr;

	// finally return value. 
    return newctr;
    }


double TEWMA::check(unsigned char *in, int len, uint64_t timestamp) {

	// Same steps as in add, but without insertion
	// first compute the hash functions via khash
    compute(in,len);

	// now computes the old accumulated value and the new one before this insertion
    double minctr=c[H(1)];

    int64_t deltat;

    for(int i=1; i<=nhash; i++) {
        deltat = timestamp - t[H(i)];
        assert(deltat >= 0);
        
        t[H(i)] = timestamp;
        c[H(i)] *= pow(beta, double(deltat)/W);
        if (c[H(i)]<minctr) minctr=c[H(i)];
    }

	// finally return value. 
    return minctr;
    }


double TEWMA::set_bits(const unsigned* bits, double quantity, uint64_t timestamp, int len)
{
  //int nbits = ( len > 0 ? len : nhash );

  split(bits, len);

  // computes the new counter decayed value
  double minctr=c[H(1)];

  int64_t deltat;

  for(int i=1; i<=nhash; i++) {

    deltat = timestamp - t[H(i)];
    assert(deltat>=0);

    t[H(i)] = timestamp;
    c[H(i)] *= pow(beta, double(deltat)/W);
    if (c[H(i)]<minctr) minctr=c[H(i)];
  }

  // now compute the new value including the insertion and update 
  // the filter by waterfilling the new counter array bins
  double newctr=minctr+quantity*logbeta;
  assert(newctr<=1.0e99);
  for(int i=1; i<=nhash; i++) if (c[H(i)]<newctr) c[H(i)]=newctr;
  
  // finally return value. 
  return newctr;
}


double TEWMA::check_bits(const unsigned* bits, uint64_t timestamp, int len)
{
  // int nbits = ( len > 0 ? len : nhash );

  split(bits, len);

  // now computes the old accumulated value and the new one before this insertion
  double minctr=c[H(1)];

  int64_t deltat;

  for(int i=1; i<=nhash; i++) {
    deltat = timestamp - t[H(i)];
    assert(deltat>=0);

    t[H(i)] = timestamp;
    c[H(i)] *= pow(beta, double(deltat)/W);

    if (c[H(i)]<minctr) minctr=c[H(i)];

    // printf("hash no => %d, value => %u\n", i, H(i));
  }

  // finally return value. 
  return minctr;
}
