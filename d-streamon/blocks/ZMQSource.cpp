#include <Block.hpp>
#include <pugixml.hpp>
#include <BlockFactory.hpp>
#include <iostream>
#include <PacketPlus.hpp>
#include <streamon/DnsHeader.hpp>
#include <streamon/DnsResponse.hpp>
#include <zmq.h>
#include <errno.h>
#include <string.h>

using namespace pugi;

namespace bm
{
    /**
     * Implements a block that captures packets from a standard pcap sniffer and injects them into BM as Packet messages
     */

    class ZMQSource: public Block
    {
        int m_gate_id;

        std::shared_ptr<MemoryBatch> m_mem_block;

        ZMQSource(const ZMQSource &) = delete;
        ZMQSource& operator=(const ZMQSource &) = delete;
    
    public:

        /**
         * @brief Constructor
         * @param name         The name of the sniffer block
         * @param active       Whether the block should be active or passive
         */ 

        ZMQSource(const std::string &name, bool active) : Block(name, true), //ignore options, sniffer must be active
                                                          m_gate_id(register_output_gate("sniffer_out")),
                                                          m_mem_block(new MemoryBatch(4096*4))
        {
            if (!active) {
                blocklog("ZMQSource must be active, ignoring configuration", log_warning);
            }
        }

        /**
         * @brief Destructor
         */
        virtual ~ZMQSource()
        {
        }


        /**
         * @brief Configures the block: defines the ZMQ publisher socket,
         * also, it opens the socket for capturing
         * @param n The configuration parameters 
         */
        virtual void _configure(const xml_node& n)
        {
		config_subscriber_socket( "tcp://localhost:5565", "" );
        }

 
        /** 
          * listens on the capture ZMQ socket, retrieves raw packets and sends them out as PacketPlus messages
          * Uses optimized allocation.
          */
        void _do_async_processing()
        {
            bool there_are_packets = true;

            uint8_t* packet_buffer = nullptr;
            size_t packet_length = 0;

            while (there_are_packets)
            {
		zmq_msg_t msg;

                			// 0) Receive packet buffer from ZMQ APIs
		init_zmq_msg_t( &msg );

		

                // 1) Take timestamp from ZMQ (is possible?)
                timespec p_tstamp;
                // p_tstamp.tv_sec;
                // p_tstamp.tv_nsec;

		recv_packet( msg, &packet_buffer, &packet_length );
			
		config_timestamp( &p_tstamp );

                // 2) Allocate a new Blockmon message using the batch allocator.
                //    in particular this will allocate a PacketPlus message that is
                //    an extension of blockmon Packet (PP contains tags).

                std::shared_ptr<const Msg> sp = alloc_msg_from_buffer<PacketPlus>(m_mem_block,    // The batch descriptor
                                                                                  packet_length,  // Batch will allocate sizeof(PacketPlus)
                                                                                                  // (for the object) but it is not enough:
                                                                                                  // need to allocate also the packet buffer,
                                                                                                  // so alloc_msg needs an additional length
                                                                                                  // mem layout: [packetplus|buffer...]

                                                                                  // From this point on, parameters of PacketPlus constructor are forwarded
                                                                                  const_buffer<uint8_t>(packet_buffer,  // Buffer + Capture length
                                                                                                        packet_length), 
                                                                                  p_tstamp,                             // Timestamp
                                                                                  packet_length);                       // Again the length :(
                // 3) Finally...
                send_out_through(std::move(sp), m_gate_id);
            }
        }

    private:

	void init_zmq_msg_t( zmq_msg_t *msg )
	{
		zmq_msg_t msg_tmp;
		int rc = 0;

		rc = zmq_msg_init(&msg_tmp);
		assert( rc == 0 );
		
		*msg = msg_tmp;
	}

	void recv_packet( zmq_msg_t msg, uint8_t** packet_buffer, size_t *packet_length )
	{
		int rc = 0;

		rc = zmq_msg_recv(&msg, subscriber_socket, ZMQ_DONTWAIT );
		assert( rc != -1 );

		*packet_buffer = (uint8_t *)zmq_msg_data( &msg );

		*packet_length = zmq_msg_size( &msg );
	}
	
	void config_timestamp( timespec *p_tstamp )
	{
		int err_getTime = 0;

		errno = 0;
		err_getTime = clock_gettime( TIMER_ABSTIME, p_tstamp );

		if ( ( err_getTime == -1 ) && ( errno != 0 ))
		{
			fprintf( stderr, "ERR ------ Error get timestamp\n");
			fprintf( stderr, "ERRNAME -- %s\n", strerror(errno));
			exit( EXIT_FAILURE );
		} 
	}
		
								/*
								 * 	Config Subscriber socket
								 *
								 */			
	void config_subscriber_socket( const char *tcp_port, const char *filter )
	{	
		int rc = 0;

		void *context = zmq_ctx_new();

		subscriber_socket = zmq_socket( context, ZMQ_SUB );

		rc = zmq_connect( subscriber_socket, tcp_port );				
		assert( rc == 0 );


		rc = zmq_setsockopt( subscriber_socket, ZMQ_SUBSCRIBE, filter, strlen(filter));
		assert( rc == 0 );
	} 



	void *subscriber_socket;

    };

    REGISTER_BLOCK(ZMQSource,"ZMQSource");
}
