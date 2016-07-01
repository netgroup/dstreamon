
#pragma once


#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "DnsHeader.hpp"


class DnsResponse
{
    // DATA STRUCTURES
    struct dns_query_name
    {
        struct
        {
            uint8_t len : 6;
            uint8_t cmp : 2;
        };

        char data[0];
    };

    // DATA MEMBERS
    const uint8_t* data;
    const uint8_t* start;

    int32_t availableBytes;

    uint16_t lastAliasOffset;
     
    std::map<uint16_t, std::string, std::greater<uint16_t> > cachedNames; // newer entry come first 

    std::vector<uint32_t> ipList;

    // FUNCTIONS
    std::string getQueryName(const uint8_t* p, uint8_t** new_data, int32_t lenHint = 0);

    void parseRecord();

    
    // CONSTRUCTORS
    DnsResponse() : data(NULL),
                    start(NULL)
    {
    }


    DnsResponse(const DnsHeader& head) : data(head.data),
                                         start(data-sizeof(struct dns_header) + sizeof(struct udp)),
                                         availableBytes(head.dnsPayloadLength),
                                         lastAliasOffset(0)
    {
        // std::cout << "DNS USEFUL DATA LEN: " << availableBytes << "\n";

        for (int i=0; i < head.questionNumber; i++)
        {
            ptrdiff_t offset = data - start;

            uint8_t* new_data = NULL;
    
            // fill the cache

            cachedNames[offset] = getQueryName(data, &new_data);

			// std::cout << "[DnsResponse] Query name: " << cachedNames[offset] << "\n";

            lastAliasOffset = offset;
 
            data = new_data + 4;

            availableBytes -= 4; // read qtype and qclass
        }

        
        // std::cout << "Read questions, available bytes: " << availableBytes << "\n";

        
        for (int i=0; i < head.answerRecordNumber; i++)
        {
            parseRecord();
        }

        // std::cout << "Read answers, available bytes: " << availableBytes << "\n";

        for (int i=0; i < head.authRecordNumber; i++)
        {
            parseRecord();

            // std::cout << "ReadING auth, available bytes: " << availableBytes << "\n";
        }

        for (int i=0; i < head.additionalRecordNumber; i++)
        {
            parseRecord();
        }

        // std::cout << "End of parsing, availableBytes: " << availableBytes << "\n";
    }

public:

	/*
    static DnsResponse parse(const uint8_t* ip_packet, int len)
    {
        const struct udp* up = reinterpret_cast<const struct udp*>(ip_packet);
        
        if (up->source_port == htons(53))
        {
            DnsHeader head = DnsHeader::parse(ip_packet, len);

            return DnsResponse(head);
        }
        else
        {
            return DnsResponse();
        }
    }
	*/


    static DnsResponse parse(const DnsHeader& head)
    {
		bool isBad = (!head || 
					  // !head.isAnswer() || 
					   head.getOpCode() != 0 || 
					   head.getReturnCode() == 1 || head.getReturnCode() > 4 ||
					   head.questionNumber != 1 );

        if (isBad)
		{
			return DnsResponse();
		}

        return DnsResponse(head);
    }

    
    bool operator!() const { return !data; }


    const std::string& getName() const
    {
        auto it = cachedNames.end();

        return (--it)->second;
    }

    const std::string& getAlias() const
    {
        auto it = cachedNames.find(lastAliasOffset);

        assert( it != cachedNames.end() );

        return it->second;
    }

    const std::vector<uint32_t>& getAddresses() const
    {
        return ipList;
    }

};
