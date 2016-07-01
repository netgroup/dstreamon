
#ifndef _PROTOCOLS_H
#define _PROTOCOLS_H

#include <stdint.h>
#include <arpa/inet.h>

struct ipv4
{
    uint8_t ver_hlen;
    uint8_t _not_used;
    uint16_t length;
    uint16_t id;
    uint16_t flags_foffset;
    uint8_t time_to_live;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t source;
    uint32_t destination;

    char payload[0];
};


struct udp
{
    struct ipv4 ip;

    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;

    char payload[0];
};


struct tcp
{
	struct ipv4 ip;

    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq_no;
    uint32_t ack_no;
    uint16_t offset_flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urg_ptr;

	char payload[0];
};


struct dns_header
{
    struct udp udp;

    uint16_t id;

    uint16_t flags;

    uint16_t total_questions;
    uint16_t total_answer_rr;
    uint16_t total_auth_rr;
    uint16_t total_additional_rr;

    uint8_t  data[0];
};


struct __attribute__((__packed__)) dns_record_attr
{    
    uint16_t type;
    uint16_t rclass;
    uint32_t ttl;
    uint16_t length;
    uint8_t  data[0];
};


void print_dns_flags(uint16_t flags);
void print_dns_header(const struct dns_header* head);


#endif /* _PROTOCOLS_H */
