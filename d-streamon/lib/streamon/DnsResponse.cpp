
#include "DnsResponse.hpp"

#define DNS_MAX_LENGTH 256

std::string DnsResponse::getQueryName(const uint8_t* p, uint8_t** new_data, int32_t lenHint)
{
    char buffer[DNS_MAX_LENGTH];
    char* end = buffer + sizeof(buffer);

    char* b = buffer;

    // std::cout << __PRETTY_FUNCTION__ << "\n";

    while (*p)
    {
        struct dns_query_name* q = (struct dns_query_name*) p;

        // check if there is message compression

        // uint16_t head = ntohs(*reinterpret_cast<const uint16_t*>(q));
 
        availableBytes -= 1; // read 1 byte length

        // uint16_t compressed_flag = ( head >> 14 ) & 0x0003;

        if (q->cmp == 0x3)
        {
            if (availableBytes < 1) break;

            availableBytes -= 1;

            // assert( q->cmp == 3 );

            uint16_t head = ntohs(*reinterpret_cast<const uint16_t*>(q));

            int32_t offset = (head & 0x3FFF);

            // search in the cache for the entry with the
            // greater index lower than offset
            
            auto base_it = cachedNames.lower_bound(offset);

			if ( base_it == cachedNames.end() ) // assert( base_it != cachedNames.end() );
			{
				throw std::runtime_error("[DnsResponse] Compressed name refers to an invalid position.");
			}

			                        
            // calculate NEW offset in string base

            offset -= base_it->first;

			assert( offset >= 0 );

            
            if (offset == 0) // the entire string must be returned, as is already cached
            {                
                *new_data = const_cast<uint8_t*>(p+2);

                return base_it->second;
            }


            // calculate length of concatenation

            int32_t length = base_it->second.size() - offset;

			if ( length <= 0 ) // assert( length > 0 );
			{
				throw std::runtime_error("[DnsResponse] Compressed name concatenation with 0-length string.");
			}

			
			if ( b + length + 1 >= end )
			{
				throw std::runtime_error("[DnsResponse] Error: compressed name length greater than buffer size? ");
			}

            // assert( b + length + 1  < end );

            memcpy(b, base_it->second.c_str()+offset, length);

            b += length;

            // as this is the last part of string, terminate and return

            *b = '\0';

            *new_data = const_cast<uint8_t*>(p+2);
            
            return std::string(buffer);
        }
        

        if ( availableBytes < q->len )
        {
            std::cout << "Available bytes: " << availableBytes << ", QLen: " << q->len << "\n";

            throw std::runtime_error("[DnsResponse] Packet buffer too short (maybe corrupted).");
        }

		if ( b + q->len + 1 >= end )
		{
			throw std::runtime_error("[DnsResponse] Error: name length greater than 128? ");
		}

        availableBytes -= q->len;

        // assert( b + q->len + 1  < end );

        memcpy(b, q->data, q->len);
        
        /*
        for (int i=0; i < length; i++)
            std::cout << b[i];

        std::cout << "\n";
        */ 
	  
        b += q->len;
	  
        *b = '.';

        b++;
	  
        p += q->len + 1;
    }
    
    /*
    if ( availableBytes < 2 )
    {
        throw std::runtime_error("[DnsResponse] Packet buffer too short (maybe corrupted).");
    }
    */

    *new_data = const_cast<uint8_t*>(++p);

    // after that, p points to the terminator

    availableBytes -= 1;

    // std::cout << "After name read cycle, remaining bytes: " << availableBytes << "\n";


    // uint16_t QTYPE
    // uint16_t QCLASS
    // 
    // so we need to jump by 4 to go to the next label, if exist

    // DIRTY PATCH FOR EMPTY NAMES
    if (b != buffer) b--;
	
    *b = 0;

    return std::string(buffer);
}


void DnsResponse::parseRecord()
{
    // std::cout << __PRETTY_FUNCTION__ << "\n";

    uint16_t offset = 0;

    const struct dns_record_attr* attributes = NULL;

    if (availableBytes < (int32_t) sizeof(struct dns_record_attr) )
    {
        // std::cout << "Available bytes are smaller than dns_record_attr: " << availableBytes << "\n";

        throw std::runtime_error("[DnsResponse] Packet buffer too short (maybe corrupted).");
    }

    uint16_t head = ntohs(*reinterpret_cast<const uint16_t*>(data));

    uint16_t compressed_flag = ( head >> 14 );

    if (compressed_flag == 0x0003)
    {
        offset = (head & 0x3FFF);

        // std::cout << "Compressed name: " << cachedNames[offset] << "\n";

        attributes = reinterpret_cast<const struct dns_record_attr*>(data+sizeof(uint16_t));

        data += sizeof(uint16_t); // now data point to attributes

        availableBytes -= sizeof(uint16_t);
    }
    else
    {
        // throw std::runtime_error("[DnsResponse] domain names without compression in Resource Record");

        offset = data - start;

        uint8_t* new_data = NULL;

        cachedNames[offset] = getQueryName(data, &new_data);

		// std::cout << "[DnsResponse] domain name without compression: " << cachedNames[offset] << "\n";

        attributes = reinterpret_cast<const struct dns_record_attr*>(new_data);

        data = new_data;
    }

    /*
    if (availableBytes < 4)
    {
        throw std::runtime_error("[DnsResponse] Packet buffer too short (maybe corrupted).");
    }
    */

    // PARSE ATTRIBUTES

    uint16_t type   = ntohs(attributes->type);
    uint16_t length = ntohs(attributes->length);

    availableBytes -= sizeof(struct dns_record_attr);


    if (availableBytes < length)
    {
        std::cout << "Available bytes: " << availableBytes << ", Attribute length: " << length << "\n";

        throw std::runtime_error("[DnsResponse] Packet too short for the attribute len.");
    }


    if ( type == 1 ) // TYPE A
    {
		if ( length != sizeof(uint32_t) )
		{
			std::string errstr = "[DnsResponse] TYPE A expects an attribute length of 4.";

			throw std::runtime_error(errstr);
		}
        // assert( length == sizeof(uint32_t) );

        ipList.reserve(8);

        const uint32_t* ipp = reinterpret_cast<const uint32_t*>(attributes->data); 

        availableBytes -= sizeof(uint32_t);

        ipList.push_back(ntohl(*ipp));
    }
    else if (type == 5 ) // TYPE CNAME
    {
		if (length == 0 || availableBytes < length)
		{
			throw std::runtime_error("[DnsResponse] CNAME length is 0.");
		}
        // assert( length > 0 );

        offset = attributes->data - start;

        uint8_t* new_data = NULL;

        cachedNames[offset] = getQueryName(attributes->data, &new_data);

		// std::cout << "CNAME: " << cachedNames[offset] << "\n";

        // assert( cachedNames[offset].size() == length - 2 );

        lastAliasOffset = offset;
    }
    else
    {
        /*
        std::cerr << "TYPE is not A or CNAME: " << type << "\n";

        std::cerr << "Data len: " << length << "\n";
        */

        availableBytes -= length;
    }

    // std::cout << "TYPE: " << type << "\n";

    data += sizeof(struct dns_record_attr) + length;

    // availableBytes -= length;
}
