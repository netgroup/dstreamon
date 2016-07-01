#include "CounterIP_S.hpp"
#include "NetTypes.hpp"
#include "Packet.hpp"
#include <netinet/in.h>

namespace bm {

	/*
	 * restituisce il pacchetto
	 */
	const Packet* CounterIP_S::getPacket() const{
	
		return  packet;
	}

	/*
	 * restituisce la mappa
	 */
	std::map<std::string,double> CounterIP_S::getMap() const{

		return mappa;
	}
	
	/*
	 * restituisce il valore
	 */
	double CounterIP_S::getValue(std::string key) {
		
		return mappa[key];
	}
	
	/*
	 * imposta il pacchetto
	 */
	void CounterIP_S::setPacket(Packet* p){

		packet = p;
	}

}
