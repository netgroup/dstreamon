/*
 * FeedPacket.h
 *
 *  Created on: Jun 14, 2013
 *      Author: markin
 */

#ifndef FEEDPACKET_H_
#define FEEDPACKET_H_

#include <iostream>
#include "pugixml.hpp"
#include <sstream>

class FeedPacket {
public:
	FeedPacket();
	virtual ~FeedPacket();

	std::string serialize();

	std::string attackType;
	std::string name;
	std:: string content;
	int status;
	bool isAlerted;
	bool parsed;
};

#endif /* FEEDPACKET_H_ */
