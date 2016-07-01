/*
 *  Socket_Unix.cpp
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */

#include "SocketCUnix.h"
#include <unistd.h>


SocketCUnix::SocketCUnix(bool nonblock) : SocketUnix(-1)
{

    if ((s = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error in Socket.\n");
        exit(1);
    }

    if (nonblock)
    {
        int flags = fcntl(s, F_GETFL, 0);
        fcntl(s, F_SETFL, flags | O_NONBLOCK);
    }

    /*server.sun_family = AF_UNIX;
    strcpy(server.sun_path, socket_name.c_str());
    len = socket_name.size() + sizeof(server.sun_family);*/
}

	
SocketCUnix::~SocketCUnix()
{
    if (close(s) == -1)
    {
        perror("Error in close.\n");
        exit(-1);
    }
}

void SocketCUnix::start_connection(std::string& socket_name)
{

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, socket_name.c_str());
    size_t len = socket_name.size() + sizeof(server.sun_family);
    if (connect(s, (struct sockaddr *)&server, len) == -1) {
        perror("Error in connect.\n");
        exit(-1);
    } 
}
