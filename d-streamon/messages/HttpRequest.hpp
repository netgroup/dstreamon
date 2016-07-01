
#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <map>
#include <sstream>
// #include "HttpPacket.hpp"
#include <streamon/protocols.h>
#include <iostream>

namespace bm
{
    enum class HttpMethod : uint32_t
    {
        UNKNOWN = 0x00000000U,
        GET  = 0x20544547U,
        HEAD = 0x44414548U,
        POST = 0x54534F50U
    };


    class HttpRequest
    {
        const char* buffer;
		char* headers;
        size_t len;

        int32_t availableLen;

        uint16_t on_port;

        HttpMethod method;

        int32_t method_len;

        std::string s_method;
        std::string uri;
        std::string args;
        std::string version;

        size_t args_len;

        char* p_args;
        char* p_version;

        int32_t version_len;

		size_t max_uri_len;

		const char* check(const char* packet, size_t ip_len)
		{
			const struct tcp* tp = reinterpret_cast<const struct tcp*>(packet);

			if (ip_len < sizeof(struct tcp)) return NULL;

			std::cout << "[HttpRequest] Protocol: " << (uint16_t) tp->ip.protocol << "\n";
			std::cout << "[HttpRequest] Dest Port: " << ntohs(tp->dest_port) << "\n";

			if (tp->ip.protocol != 6 || (on_port && tp->dest_port != on_port)) return NULL;

			buffer += sizeof(struct tcp);

			return tp->payload;
		}


        public:

        HttpRequest(const uint8_t* _buffer, size_t _len) : //, uint16_t expected_port = 80) :
            buffer((const char*)_buffer),
			headers(NULL),
            len(_len),
            availableLen(_len),
            // on_port( htons(expected_port) ),
            method(HttpMethod::UNKNOWN),
            method_len(4),
            s_method(),
            uri(),
            args(),
            args_len(0),
            p_args(NULL),
            p_version(NULL),
			max_uri_len(128)
        {
        }

		std::string getHeader(const char* name);

        bool parse();

        HttpMethod getMethod() { parse(); return method; }

        std::string getUri() { parse(); return uri; }
 
        std::string getAbsoluteUri()
        {
            return getHeader("Host") + getUri(); 
        }

        std::string getMethodString()
        {
            if (parse() && s_method.empty())
            {
                s_method = std::string( (const char*) buffer, method_len);
            }

            return s_method;
        }

        
        std::string getParamString()
        {
            if (parse() && args.empty())
            {
                args = std::string(p_args, args_len);
            }

            return args; 
        }

        std::string getVersion()
        {
            if (parse() && version.empty())
            {
                version = std::string(p_version, version_len);
            }

            return version;
        }

		void setMaxUriLen(size_t limit)
		{
			max_uri_len = limit;
		}
    };
}
