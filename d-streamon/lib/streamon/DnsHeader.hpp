
#pragma once

#include <cassert>

#include "protocols.h" 

class DnsHeader
{

public:

    const uint16_t id;
    const uint16_t flags;
    const uint16_t questionNumber;
    const uint16_t answerRecordNumber;
    const uint16_t authRecordNumber;
    const uint16_t additionalRecordNumber;

    const uint8_t* data;

    const int32_t  dnsPayloadLength;

private:

    DnsHeader() : id(0),
                  flags(0),
                  questionNumber(0),
                  answerRecordNumber(0),
                  authRecordNumber(0),
                  additionalRecordNumber(0),
                  data(NULL),
                  dnsPayloadLength(0)
    {
    }


    DnsHeader(const struct dns_header* head, int32_t dns_payload_len) : id( ntohs(head->id) ),
                                                                        flags( ntohs(head->flags) ),
                                                                        questionNumber( ntohs(head->total_questions) ),
                                                                        answerRecordNumber( ntohs(head->total_answer_rr) ),
                                                                        authRecordNumber( ntohs(head->total_auth_rr) ),
                                                                        additionalRecordNumber( ntohs(head->total_additional_rr) ),
                                                                        data(head->data),
                                                                        dnsPayloadLength(dns_payload_len)
    {
        // std::cout << "[DnsHeader] sizeof => " << sizeof(struct dns_header)-sizeof(struct udp) << "\n";

        // actually handle the simple case

        // assert(questionNumber == 1);
    }


public:

    static DnsHeader parse(const uint8_t* ip_packet, size_t ip_len)
    {
        static const uint16_t DNS_PORT = htons(53);

		assert(ip_packet);
        
        const struct udp* up = reinterpret_cast<const struct udp*>(ip_packet);

        // FAST LENGTH CHECK

        if ( ip_len < sizeof(struct dns_header) ) return DnsHeader();


        // CHECK IF IPv4 UDP
        
        auto ip_ver = (up->ip.ver_hlen >> 4);

        if (ip_ver != 4 || up->ip.protocol != 17)
        {
            // std::cout << "[DnsHeader] DROP: not IPv4 UDP\n";

            return DnsHeader();
        }

        // CHECK PORTS

        if (up->source_port != DNS_PORT && up->dest_port != DNS_PORT)
        {
            // std::cout << "[DnsHeader] DROP: not port 53\n";

            return DnsHeader();
        }

        // OK IS UDP PORT 53, NOW PERFORM LENGTH COHERENCE CHECK

        if ( ip_len != sizeof(struct ipv4) + ntohs(up->length) )
        {
            // std::cout << "[DnsHeader] SIZE ERROR, DISCARD\n";

            return DnsHeader();
        }

        
        return DnsHeader( reinterpret_cast<const struct dns_header*>(ip_packet), ip_len - sizeof(struct dns_header) );
    }


    bool isAnswer() const
    {
        return (flags >> 15);
    }


    uint16_t getOpCode() const
    {
        return (flags >> 11) & 0x000F;
    }

    uint16_t getReturnCode() const
    {
        return flags & 0x000F;
    }


    bool operator!() const { return !data; }


    friend std::ostream& operator<<(std::ostream& stream, const DnsHeader& head)
    {
        stream << "*** DnsHeader ***\n"
               << "ID     : " << std::hex << head.id << "\n"
               << "FLAGS  : " << std::hex << head.flags << std::dec << "\n"
               << "QUEST  : " << head.questionNumber << "\n"
               << "ANSWERR: " << head.answerRecordNumber << "\n"
               << "AUTHRR : " << head.authRecordNumber << "\n"
               << "ADDIRR : " << head.additionalRecordNumber << "\n\n"
               << "DNS PAYLOAD LEN: " << head.dnsPayloadLength << "\n"
               << "*****************\n\n";

        return stream;
    }
};
