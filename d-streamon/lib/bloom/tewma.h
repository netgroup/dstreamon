#ifndef _TEWMA_H
#define _TEWMA_H
#include "khash.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

/***********************************************************************************

	TEWMA: implements a smoothed rate meter using a CBF and a timestamp array
	it follows the TEWMA rules
	Time is measured according to a window reference passed in the constructor:
	use _w=1 if you wish to stick to seconds.

	Constructor:
		_nhash		: number of filter hash
		_shash		: digest log size for cbf (e.g.: 6 means 2^6 = 64)
		_beta		: smoothing parameter

 ***********************************************************************************/


class TEWMA : public khash {

	// specific variables for the rmeterfp class
    uint64_t* t;		// timestamp array
    double*	  c;		// counter array

	// smoothing parameter
    double	beta;
    double	logbeta;

	// time unit (in seconds)
    double	W;

	// variables inherited from khash
	//     	nhash;
	//	shash;


    public:
			// basic functions (constructor, destructor, etc)
		TEWMA(int _nhash, int _shash, double _beta, double _w);
	virtual ~TEWMA();
	void	clear(uint64_t timestamp_start);
	void	set_beta(double b);

			// core functions: add, check, step
	double 	add(unsigned char *in, int len, double quantity, uint64_t timestamp);
	double 	check(unsigned char *in, int len, uint64_t timestamp);

	double set_bits(const unsigned* bits, double quantity, uint64_t timestamp, int len = 0);
	double check_bits(const unsigned* bits, uint64_t timestamp, int len = 0);

    };

#endif
