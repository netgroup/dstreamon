#include "KeyFlag.hpp"
#include "NetTypes.hpp"
#include <netinet/in.h>
#include "lib_botstream/cbf.hpp"
namespace bm {

	/*
	 * restituisce il counting bloom filter
	 */
	CBF KeyFlag:: getCBF() const{
	
		return  countBF;
	}
		
	/*
	 * restituisce il vector di string
	 */
	std::vector<std::string> KeyFlag:: getIP() const{
	
		return ip_src;
	}

	/*
	 * restituisce la finestra temporale
	 */
	int KeyFlag:: getWindow() const{
	
		return window;
	}
}
