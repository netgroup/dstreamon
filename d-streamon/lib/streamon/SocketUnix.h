/*
 *  SocketUnix.h
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */

#pragma once

#include <cstdlib>
#include <string>

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

class SocketUnix
{

protected:
    	int s;
	SocketUnix(int);
public:
    int receive(char*, int);
    int send(const char*, int);

};
