/*
 *  SocketSUnix.h
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */

#pragma once

#include "SocketUnix.h"

class SocketSUnix : public SocketUnix {
	
	
	// int s;
	struct sockaddr_un client;
public:
	
	SocketSUnix(std::string);
	
};
