// FIXME brian Redocument this; change the API
// FIXME add on-demand parsing of IP header, ports, and TCP header fields
// don't use five-tuple anymore
// add IPv6 support (FUTURE)

/**
 * @file 
 *
 * This message represents a Packet.
 */
 
// FIXME determine whether timespec is the right way to go

/* Idem hack */
#ifndef _KeyFlag_HPP_
#define _KeyFlag_HPP_ 

#include "Msg.hpp"
#include "Buffer.hpp"
#include "MemoryBatch.hpp"
#include "ClassId.hpp"
#include "MsgRegistry.hpp"
#include "NetTypes.hpp"
#include <cstring>
#include <cassert>
#include <ctime>
#include "lib_botstream/cbf.hpp"
namespace bm
{

    /**
     * KeyFlag Msg: rappresenta un messaggio contenente un counting bloom filter e un vector di indirizzi IP
     */

    class KeyFlag : public Msg
    {
      
    public:


	KeyFlag(int w, CBF cbf, std::vector<std::string> ip):Msg(MSG_ID(KeyFlag)),
	window (w),
	countBF(cbf),
	ip_src(ip)
	{}

	
        /**
        * Destroy this Packet. Frees the packet buffer if owned.
        */
        virtual ~KeyFlag()
        {
           
        }

	CBF getCBF() const;

	std::vector<std::string> getIP() const;

	int getWindow() const;




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Import/export via IPFIX
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
        struct pod_struct
        {
            uint64_t            ms;
            size_t              len;
            IPFIX::VarlenField  buf;
        }; 
        

class Descriptor: public Msg::Descriptor
        {
        public:
           
        /**
             * Construct a new Msg from the POD representation returned by
             * copy_pod for the given Msg.
             *
             * @param buf buffer containing the POD representation
             */
	virtual const Msg& create_from_pod(const_buffer<uint8_t>& buf){

        static KeyFlag kf(0, CBF(262144, 4), std::vector<std::string>());
		
		// int w = 0;
		// CBF cbf(262144, 4);
		// std::vector<std::string> ip;
                
		return kf;
    }
        
		 /**
             * Given a reference to an empty IETemplate, fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param st template to fill
             */
            virtual void fill_pod_template (IPFIX::StructTemplate& st) const {
                IPFIX::InfoModel& model = IPFIX::InfoModel::instance();
                st.add(model.lookupIE("observationTimeMilliseconds"), offsetof(pod_struct, len));
                st.add(model.lookupIE("ipTotalLength")->forLen(sizeof(size_t)), offsetof(pod_struct, len));
                st.add(model.lookupIE("ipPacketHeaderSection"), offsetof(pod_struct, buf));
                st.activate();
            }
            /**
             * Given a reference to an empty IETemplate (a WireTemplate returned
             * from IPFIX::Session.getTemplate()), fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param wt template to fill
             */
      
		virtual void fill_wire_template (IPFIX::WireTemplate& wt) const {
                IPFIX::InfoModel& model = IPFIX::InfoModel::instance();
                wt.add(model.lookupIE("observationTimeMilliseconds"));
                wt.add(model.lookupIE("ipTotalLength[2]"));
                wt.add(model.lookupIE("ipPacketHeaderSection"));
            }
        };

	private:
		
		int m_type;
		int window;
		CBF countBF;	
		std::vector<std::string> ip_src;
		

    };

	REGISTER_MESSAGE_CLASS(KeyFlag,KeyFlag::Descriptor)
}


#endif /* _KeyFlag_HPP_ */
