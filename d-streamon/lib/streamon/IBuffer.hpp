
/* ###########################################
 *
 * IREADBUFFER
 *
 * This interface will provide base to
 * implement buffer classes, offering
 * GetLength() and GetRawData() methods
 * 
 * The interface also inherit from IHashable
 * that expose GetHash() method
 * 
 * The programmer need to implement both
 * GetRawData() and GetHash() methods in
 * the concrete classes
 *
 * 
 * Author: Giulio Picierro, CNIT
 *
 * Mail: giulio.picierro@uniroma2.it
 * 
 * Version: 2.0 (introduced IHashable iface)
 *
 * ##########################################
 */

#pragma once
#include "IHashable.hpp"
#include <string>
#include <sstream>
#include <stdexcept>
#include <arpa/inet.h>


#define __BUFFER_ERROR_STR "Buffer full."
#define __BUFFER_ERROR_GET "Not enough data to fetch buffer"

enum struct buffer_type : uint16_t
{
    UNKNOWN,
    UINT,
    IPv4,
    DOUBLE,
    STRING

};


class IReadBuffer : public IHashable
{

private:

    std::string ip_to_string(uint32_t ip) const
    {
        char addr_buffer[INET_ADDRSTRLEN];

        /*
         *inet_ntop expects network byte order
         */

        uint32_t flipped_ip=htonl(ip);

        if(!inet_ntop(AF_INET, &flipped_ip, addr_buffer, INET_ADDRSTRLEN))
        {
            throw std::runtime_error("cannot convert ip address");
        }

        return std::string (addr_buffer);
    }


protected:

    buffer_type Type;

    uint16_t Count;
    
public:

    IReadBuffer() : Type(buffer_type::UNKNOWN), Count(0) {}


    size_t GetLength() const
    {
        return Count;
    }


    buffer_type GetType() const
    {
        return Type;
    }

    virtual const unsigned char* GetRawData() const = 0;


    template<typename T>
    T Get(unsigned long Offset = 0) const
    {
        if ( Count - Offset < sizeof(T) )
        {
            // std::cout << "Get(): Buffer len " << Count << ".\n";
            throw std::out_of_range(__BUFFER_ERROR_GET);
        }

        T Value = *(T*) (GetRawData() + Offset);

        return Value;
    }


    std::string GetString() const
    {
        // this function offer pretty-print
        // services for i.e. IP address (standard dot notation)
			
        std::string output;

        if (Count == 0 || !GetRawData()) return output;
			
        switch (Type)
        {
            case buffer_type::UINT:
                {
                    uint32_t int_field = *(uint32_t*)GetRawData();
                    
                    std::stringstream ss;
                    ss << int_field;
                    output = ss.str(); 

                    break;
                }
            case buffer_type::IPv4:
                {                    
                    uint32_t ip_field = *(uint32_t*)GetRawData();
                    output = ip_to_string(ip_field);
                    
                    break;
                }
            case buffer_type::DOUBLE:
                {
                    std::stringstream ss;
                    ss << Get<double>();

                    output = ss.str();

                    break; 
                }
            case buffer_type::STRING:
                {
                    output = std::string( (const char*) GetRawData(), Count );
                    break;
                }
            default:
                {
                    // std::cout << "Type unknown."<< std::endl;

                    output = std::string( (const char*) GetRawData(), Count );
                    break;
                }
        }
			
        return output;
    }
};
