/*
 *  SocketSUnix.cpp
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */

#include "SocketSUnix.h"
#include <unistd.h>

SocketSUnix::SocketSUnix(std::string socket_name) : SocketUnix(-1)
{

    int len = 0;

    if ((s = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error in Socket.\n");
        exit(1);
    }

    client.sun_family = AF_UNIX;
    strcpy(client.sun_path, socket_name.c_str());
    unlink(client.sun_path);
    len = socket_name.size() + sizeof(client.sun_family);

    if (bind(s, (struct sockaddr *)&client, len) == -1)
    {
        perror("Error in Bind.\n");
        exit(1);
    }

}
