
#include "HttpRequest.hpp"
#include <iostream>
#include <cassert>

namespace bm
{
    std::string HttpRequest::getHeader(const char* name)
    {
        char* buff = NULL;
        char* end  = NULL;
        int32_t len = 0;

        if ( !parse() ) goto out;

        len = strlen(name);

        if (len == 0 || availableLen < len) {
            throw std::runtime_error("[HttpRequest] getHeader: not enough bytes to read header");
            goto out;
        }

        // boyer-more search algorithm

        buff = strstr(headers, name);

        if (buff && buff[len] == ':')
        {
            buff += len;

            while ( *(++buff) == ' ' );

            // now find the len

            end = strchr(buff, '\r');

            if (end && end[1] == '\n')
            {
                return std::string(buff, end-buff);
            }
        }

    out:
        return "";
    }





    bool HttpRequest::parse()
    {
        // fast return if already parsed well
        if (headers) return true;

		// here do protocols check
        if (len == 0) return false;

		// if ( !check(buffer, len) ) return false;

		// if (buffer == NULL) return false;

		// printf("[HttpRequest] Buffer:\n%s\n\n", buffer);

        if (availableLen < method_len) {
            throw std::runtime_error("[HttpRequest] Not enough bytes to parse method.");
            return false;
        }

        method = *reinterpret_cast<const HttpMethod*>(buffer);

		// printf("[HttpRequest] Method: %02x\n", method);

		// std::string methodStr(buffer, method_len);

		// std::cout << "[HttpRequest] MethodStr: " << methodStr << "\n";

        switch (method)
        {
            case HttpMethod::GET:
                method_len--;
                break;
            case HttpMethod::HEAD:
                break;
            case HttpMethod::POST:
                break;
            default:
                method = HttpMethod::UNKNOWN;
                return false;
        }


        // parse uri
        const char* url = buffer+method_len+1;

        char* ptr = (char*)url;

        size_t uri_len = 0;

		// std::cout << "[HttpRequest] Uri: ";

        while ( availableLen > 0 && *ptr != ' ' )
        {
            // std::cout << *ptr;

            if (*ptr == '?')
            {
                // args in url
                p_args = ptr+1;
            }

            uri_len++;
            ptr++;
            --availableLen;
        }

        // std::cout << "\n";

        if (availableLen <= 0) {
            throw std::runtime_error("[HttpRequest] Not enough bytes to parse uri.");
            return false;
        }

        args_len = ptr-p_args;

		uri_len = std::min(uri_len, max_uri_len);

        if (uri_len == 0) {
            assert(*ptr == ' ');
            throw std::runtime_error("[HttpRequest] URI Length 0");
            return false;
        }

        uri = std::string(url, uri_len);

		// std::cout << "[HttpRequest] Uri: " << uri << "\n";

        // parse version
        p_version = ptr+1;

        headers = strchr(ptr, '\r');

        version_len = headers - p_version;

        if (availableLen < version_len) { 
            throw std::runtime_error("[HttpRequest] Not enough bytes to parse version.");
            return false;
        }

        availableLen -= version_len;

        if (headers && headers[1] == '\n') headers+=2;

        return true;
    }


    /*
    std::map<std::string, std::string> getRequestParameters()
    {
        // parse the string formed like:
        // field1=content1+content2&field2=param2

        std::map<std::string, std::string> params;

        // be sure that main headers is parsed
        if ( !parse() ) return params;

        char* base = args;
        char* cur = base;

        int count = 0;

        std::string key;

        while (*cur)
        {
            if (*cur == '=')
            {
                key.assign(base, count);

                base = cur+1;

                count = 0;
            }
            else if (*cur == '&')
            {
                params[key] = std::string(base, count);

                base = cur+1;

                count = 0;
            }
            else
            {
                count++;
            }

            cur++;
        }

        // insert last element (if exist)
        if (count > 0)
            params[key] = std::string(base, count);

        return params;
    }
    */    
}
