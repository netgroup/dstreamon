/*
 *  SocketUnix.cpp
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */

#include "SocketUnix.h"
#include <stdexcept>
#include <unistd.h>

SocketUnix::SocketUnix(int id){

	s = id;
}

int SocketUnix::send(const char* buffer, int len){

	int leng = 0;
	if ((leng = write(s, buffer, len)) == -1)
	{
		if (errno != EAGAIN)
		{
			char errbuff[256];

			strerror_r(errno, errbuff, 256);

			throw std::runtime_error(errbuff); // perror("Error in Write:");
		}
	}

	return leng;
}

int SocketUnix::receive(char* buffer, int len){

	int leng = 0;
	
	if((leng = read(s, buffer, len)) < 0){
		perror("Error in read.\n");
		exit(1);
	}

	return leng;
}
