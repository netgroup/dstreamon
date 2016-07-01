/*
 * FeedPacket.cpp
 *
 *  Created on: Jun 14, 2013
 *      Author: markin
 */

#include "FeedPacket.h"

FeedPacket::FeedPacket()
: parsed(false)
{
	// TODO Auto-generated constructor stub

}

FeedPacket::~FeedPacket() {
	// TODO Auto-generated destructor stub
}

std::string FeedPacket::serialize()
{
	std::cout << "Sending content: " << content << std::endl;	
	pugi::xml_document doc;
	pugi::xml_node feed = doc.append_child("feed");
	feed.append_attribute("name").set_value(name.c_str());
	feed.append_attribute("attack_type").set_value(attackType.c_str());
	feed.append_attribute("content").set_value(content.c_str());
	feed.append_attribute("alert").set_value(isAlerted);

	std::ostringstream o;
	doc.save(o, "\t", pugi::format_raw);

	return o.str();
}
