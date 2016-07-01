#pragma once

#include "streamon/protocols.h" 




struct  __attribute__((__packed__)) modbus  
{
	struct tcp;
	
	uint16_t transaction_identifier;
	uint16_t protocol_identifier;
	uint16_t length;
	uint8_t  unit_identifier;
	uint8_t  function_code;
	uint8_t  data[0];

};


class Modbus
{
public:
	const uint16_t transaction_identifier;
	const uint16_t protocol_identifier;
	const uint16_t length;
	const uint8_t  unit_identifier;
	const uint8_t  function_code;
	
	const uint8_t  *data;
	
	const int32_t  modbusPayloadLength;
	
	
private:
	Modbus() :  transaction_identifier(0),
				protocol_identifier(0),
				length(0),
				unit_identifier(0),
				function_code(0),
				data(NULL),
				modbusPayloadLength(0)
	{
	}
				
			 
	Modbus(const struct modbus* mod,int32_t modbus_payload_len):        transaction_identifier( ntohs(mod->transaction_identifier) ),
                                                                        protocol_identifier( ntohs(mod->protocol_identifier) ),
                                                                        length( ntohs(mod->length) ),
                                                                        unit_identifier(mod->unit_identifier) ,
                                                                        function_code(mod->function_code ),
                                                                        data(mod->data),
                                                                        modbusPayloadLength(modbus_payload_len)
    {
	}
				
	
	public:
			static Modbus parse( const uint8_t* tcp_packet, size_t tcp_len)
			{
				
				if ( tcp_len < sizeof(struct modbus) ) return Modbus() ;
				
				return Modbus( reinterpret_cast<const struct modbus*>(tcp_packet), tcp_len - sizeof(struct modbus) );
			}
	
	
	
		uint16_t get_ti() const
        {
			return transaction_identifier;
        }
			
		uint16_t get_pi() const
        {
			return protocol_identifier;
         }
    
		uint16_t get_l() const
		{
			return length; 
		}
    
		uint8_t get_ui() const
		{
			return unit_identifier;
		}
    
		uint8_t get_fc() const
		{
			return function_code;
		}	
	
};
























