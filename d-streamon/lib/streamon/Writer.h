/*
 *  writer.h
 *  
 *
 *  Created by Marco Sgorlon on 17/10/12.
 *  Copyright 2012 Università Degli Studi Di Roma Tor Vergata. All rights reserved.
 *
 */



#ifndef WRITER_H
#define WRITER_H

#include "pugixml.hpp"

//static xml;
struct xml_memory_writer: pugi::xml_writer
{
    char* buffer;
    size_t capacity;
	
    size_t result;
	
    xml_memory_writer(): buffer(0), capacity(0), result(0)
    {
    }
	
    xml_memory_writer(char* buffer, size_t capacity): buffer(buffer), capacity(capacity), result(0)
    {
    }
	
    size_t written_size() const
    {
        return result < capacity ? result : capacity;
    }
	
    virtual void write(const void* data, size_t size)
    {
        if (result < capacity)
        {
            size_t chunk = (capacity - result < size) ? capacity - result : size;
			
            memcpy(buffer + result, data, chunk);
        }
		
        result += size;
    }
};


#endif
