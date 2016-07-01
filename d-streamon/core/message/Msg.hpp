/**
 * Msg.hpp
 *
 * Base class for BlockMon messages. Models a message containing no data 
 * except its type. You probably want to use an existing derived class of
 * Msg, or create your own.
 *
 * HOW TO BUILD A MESSAGE
 *
 * brian will write this on the train while wearing noise-cancelling headphones 
 * so he can ignore the boring conversation that guy is having about when 
 * he will arrive in Basel.
 *
 * Copyright notice goes here.
 */

// Make messages noncopyable

/* Idem hack */
#ifndef _MSG_HPP_
#define _MSG_HPP_

#include <string>
#include <stdexcept>
#include <memory>
#include <cstring>
#include "Buffer.hpp"
#include "WireTemplate.h"
#include "StructTemplate.h"
#include "InfoModel.h"

namespace bm { 

    /**
     * constructor tag type (a la boost, see for example lock_guard)
     */
    namespace 
    {
        struct memory_not_owned_t {};
        memory_not_owned_t memory_not_owned = memory_not_owned_t();
    }

    /**
     * Pure virtual factory superclass for building messages from POD,
     * used in IPFIX deserialization by IPFIXSource.
     * Maintains struct and wire templates for IPFIX serialization 
     * by IPFIXExporter.
     */
    
    /**
     * A BlockMon Message. (FIXME more here about messages)
     */
     
    class Msg {

    public:

        /**
         * Create a new message of a given type. 
         *
         * Type is simply an integer. Derived classes may set the private
         * m_type field in their own constructors, if they represent only 
         * one type of message.
         *
         * @param type message type number
         */
        Msg(int type)
        : m_type(type)
        {}

        // noncopyable
        Msg(const Msg&) = delete;
        Msg(const Msg&&) = delete;
        Msg& operator=(const Msg&) = delete;

        /**
         * Destroy this message and free its storage (destructor)
         *
         * Does nothing in the base class; derived classes must free all 
         * dynamically-created members.
         */
        virtual ~Msg()
        {}

        /**
         * Exchange this message's content with the content of another message
         *
         * In the base class, this simply swaps the message type; derived
         * classes should swap all members.
         *
         * @param rhs message to swap content with
         */
        void swap(Msg &rhs)
        {
            std::swap(m_type, rhs.m_type);            
        }

        /**
         * Return the type of the message.
         *
         * The type facilitiates the runtime separation of messages
         * based on type.
         *
         * @return message type
         */

        int type() const
        {
            return m_type;
        }

        /**
         * Create a clone of this message.
         *
         * This is a pure virtual function, and must be implemented by derived
         * classes.
         */
        virtual std::shared_ptr<Msg> clone() const
        {
            return std::make_shared<Msg>(m_type);
        }

        /**
         * Copy the content of this message as a C struct (Plain Old Data)
         * to a mutable buffer. May throw std::runtime_error if the buffer
         * is too small (the buffer should be at least raw_size() bytes).
         *
         * @param buf buffer to copy
         *
         */
        virtual void pod_copy(mutable_buffer<uint8_t> buf) const 
        {
            assert_pod_size(buf.len());
            memcpy(buf.addr(), &this->m_type, sizeof(int));
        }

        virtual size_t pod_size() const {
            return sizeof(int);
        }

       class Descriptor {
           const uint16_t m_template_id;
        
        public:
           /*
            * Keeps a class-wide index of the last assigned template ids: it is used to assign consecutive ids to all of the registered messages
            * Notice that, for IPFIX export, templates do not need to be consistent across different nodes
            * @return the template id assigned to the next message
            */   

           static uint16_t next_template_id()
           {
              
               static uint16_t last_id=256;
               //as registration happens when the main thread starts up, there is no concurrent access
               if(!last_id)//wrap around
                   throw std::runtime_error("blockmon ran out of template ids");
               return ++last_id;
           }

            /**
             * Returns the template id, which is computed once for all by the registry when the message type is registered
             * The registry guarantees no collisions among template ids
             * @return the template id
             */
            uint16_t template_id() const
            {
                return m_template_id;
            }

            /**
              * class constructor: automatically obtains a template id from the static class function
              */  
            Descriptor():
            m_template_id( next_template_id() )
            {}

            /**
             * Construct a new Msg from the POD representation returned by
             * copy_pod for the given Msg.
             *
             * @param buf buffer containing the POD representation
             */
            virtual const Msg& create_from_pod(const_buffer<uint8_t>& buf) = 0;
        
            /**
             * Given a reference to an empty IETemplate, fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param st template to fill
             */
            virtual void fill_pod_template (IPFIX::StructTemplate& st) const = 0;
        
            /**
             * Given a reference to an empty IETemplate (a WireTemplate returned
             * from IPFIX::Session.getTemplate()), fill the template
             * with IEs representing the POD representation of this Message.
             *
             * @param wt template to fill
             */
            virtual void fill_wire_template (IPFIX::WireTemplate& wt) const = 0;

            std::shared_ptr<IPFIX::StructTemplate> get_pod_template() const {
                auto st = std::make_shared<IPFIX::StructTemplate>();
                fill_pod_template(*st);
                return std::move(st);
            }

        };

    protected:
        void assert_pod_size(size_t len) const {
            if (len < pod_size()) {
                throw std::runtime_error("buffer too small for pod_copy/create_from_pod()");
            }            
        }

    private:
        int m_type;
    };

} // namespace bm

/* end idem hack */
#endif 
