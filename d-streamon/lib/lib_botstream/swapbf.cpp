#include <vector>
#include <cassert>
#include <iostream>
#include "hash/khash.hpp"
#include "hash/sha1.hpp"
#include "bloomfilter.hpp"
#include "swapbf.hpp"

namespace bm {

	SwapBF::SwapBF(unsigned int _size, unsigned int _nhash):
	sbf1(_size, _nhash),
	sbf2(_size, _nhash),
	activeBF(1)
	{}

	void SwapBF::insert(unsigned char* element, int len) {

		switch(activeBF) {
			case 1: {
				if(!(sbf1.isPresent(element,len))) {
					sbf1.insert(element,len);
					sbf2.insert(element,len);
					
					//std::cout << "\nactiveBF = 1, element not present\n";
				}
				
				else {
					//std::cout<< "\nelement already present in the BF1\n";
				}
			
				if(sbf2.stats_sbf()) {
					//std::cout<< "\n\n -----SYSTEM SWAPPING!----- \n";				
					activeBF = 2;
					sbf1.reset();
				}
			}
			break;
			
			case 2: {
				if(!(sbf2.isPresent(element,len))) {
					sbf1.insert(element,len);
					sbf2.insert(element,len);
					
					//std::cout << "\nactiveBF = 2, element not present\n";
				}
				
				else {
					//std::cout << "\nelement already present in the BF2\n";
				}
				
				if(sbf1.stats_sbf()) {
					//std::cout << "\n\n -----SYSTEM SWAPPING!----- \n";
					activeBF = 1;
					sbf2.reset();
				}
			}
			break;
			
			default: {
				std::cout << "\nsomething gone wrong\n";
			}
		}
	} // end bloompair_insert
	
	bool SwapBF::isPresent(unsigned char* element, int len) {
		bool isPresent = false;
		if(sbf1.isPresent(element,len) or sbf2.isPresent(element,len)) {
			isPresent = true;
			//std::cout << "\nelement already present in the SBFs\n";			
		}
		return isPresent;
	}
	
	SwapBF::~SwapBF() {}
	
}
