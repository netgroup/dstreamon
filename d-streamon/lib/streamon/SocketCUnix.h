/*
 *  Socket_Unix.h
 *  
 *
 *  Created by Marco Sgorlon on 16/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */


#include "SocketUnix.h"

class SocketCUnix : public SocketUnix
{

    struct sockaddr_un server;

    public:

    SocketCUnix(bool nonblock = false);
     
    virtual ~SocketCUnix();

    void start_connection(std::string&);
};
