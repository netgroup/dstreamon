#ifndef SWAPBF_HPP
#define SWAPBF_HPP


#include <vector>
#include <cassert>
#include "hash/khash.hpp"
#include "hash/sha1.hpp"
#include "bloomfilter.hpp"

namespace bm {

class SwapBF {

	public:
		/*
		 * this constructor call the general constructor setting divide to
		 * default value false
		 * @param _size size (bit) of the bloom filter
		 * @param _nshash number of hash functions to use
		 *
		 * _size must be 2^k for some k
		 */
		SwapBF(unsigned int _size, unsigned int _nhash);
		
		/*
		 * destructor
		 */
		~SwapBF();

		void insert(unsigned char* element, int len);

		bool isPresent(unsigned char* element, int len);

		bool stats_sbf();

	private:

		BLOOMFILTER sbf1;
		BLOOMFILTER sbf2;
		int activeBF;
	};
}

#endif
