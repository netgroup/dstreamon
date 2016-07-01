
/* ###########################################
 *
 * READBUFFER
 *
 * A simple wrapper around common data types
 *
 * NOTE: as not owned vars can be deleted,
 *       the user must care about this when
 *       access to raw bytes
 *
 * 
 * Author: Giulio Picierro, CNIT
 *
 * Mail: giulio.picierro@uniroma2.it
 *
 * Version: 2.0 (introduced bind function)
 *
 * ##########################################
 */

#pragma once

#include <string>
#include <type_traits>

#include <cstring>
#include <stdint.h>

#include "IBuffer.hpp"


class ReadBuffer : public IReadBuffer
{

protected:

  unsigned char* Data;


public:

    ReadBuffer() : Data(NULL) { }


    template< typename T >
    ReadBuffer(const T& var) 
    {        
        bind(var);
    }

    
    template< typename T >
    void bind(const T& var)
    {
        if ( std::is_integral<T>::value ) Type = buffer_type::UINT;
        else if ( std::is_floating_point<T>::value ) Type = buffer_type::DOUBLE;

        Data = (unsigned char*) &var;
        Count = sizeof(var);
    }

    void bind(const char* var)
    {
        Type = buffer_type::STRING;

        Data = (unsigned char*) var;
        Count = strlen(var);
    }

    void bind(const std::string& var)
    {
        Type = buffer_type::STRING;

        Data = (unsigned char*) var.c_str();
        Count = var.size();
    }


    virtual const unsigned char* GetRawData() const
    {
        return Data;
    }


    virtual Digest GetHash() const
    {
        throw std::runtime_error("[ReadBuffer] GetHash() not implemented!\n");
    }

};
