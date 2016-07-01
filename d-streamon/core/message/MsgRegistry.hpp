

#ifndef __POD_FACTORY_H_
#define __POD_FACTORY_H_

#include<Msg.hpp>
#include<map>
#include<vector>
#include<memory>
#include<mutex>
#include<ClassId.hpp>

namespace bm
{
    class MsgRegistry
    {

        std::map<int/*msg id*/,std::shared_ptr<Msg::Descriptor> > m_descriptors_map;

        MsgRegistry():
        m_descriptors_map()
        {}

    public:
        /**
         *Meyer's singleton idiom
         */
        static MsgRegistry& instance()
        {
            static MsgRegistry the_registry;
            return the_registry;
        }  

        /**
         * Regiters the module for a message type.  may be called mutiple times but registration only happens once.
         *
         * FIXME rewrite this documentation
         *
         * @param msg_id the id of the message class as instantiated by ClassId
         * @param podfactory a unique pointer to the subclass of Descriptor associated with this message class
         */

        void register_message_class(int msg_id, std::shared_ptr<Msg::Descriptor>&& descriptor)
        {
            if (m_descriptors_map.find(msg_id) != m_descriptors_map.end())
                return;
            // the model may be registered several times
            m_descriptors_map[msg_id] =  std::move(descriptor);
        }

        /**
         * Returns a reference to the registry entry  associated to a given message type
         * @param msg_id the id of the message class as instantiated by ClassId
         */
        const Msg::Descriptor& get_message_descriptor(int msg_id) const
        {
            auto mod_iterator = m_descriptors_map.find(msg_id); 
            if (mod_iterator == m_descriptors_map.end())
                throw std::runtime_error("Descriptor not registered");
            return *(mod_iterator->second);
        }

        /**
         * Retrieves the full list of the registered msg ids.
         * @return a vector containing all of the keys in the registry
         */
        std::vector<int> available_msg_ids()
        {
            std::vector<int> ret;
            for(auto it = m_descriptors_map.begin(); it != m_descriptors_map.end(); ++it)
            {
                ret.push_back(it->first);
            }
            return ret;
        }



    };

    /**
     * when an instance of this class is built, it automatically registers to the factory a module.
     * @param MessageClass the class of the associated message (needed in order to compute the message id)
     * @param DescriptorClass the associated message descriptor class inheriting from Msg::MessageDescriptor
     */


    template <typename MessageClass, typename DescriptorClass>
    class MsgRegistration
    {
    public:
        MsgRegistration()
        {
            MsgRegistry::instance().register_message_class(
                type_to_id<MessageClass>::id(),
                std::shared_ptr<Msg::Descriptor>(new DescriptorClass()));
        }
    };
}//namespace bm

/**
  * this macro instantiates the proper registering object and should be inserted at the bottom of the message header file
  * @param MessageClass the class of the associated message (needed in order to compute the message id)
  * @param DescriptorClass the associated message descriptor class inheriting from Msg::MessageDescriptor
  */

#define REGISTER_MESSAGE_CLASS(MessageClass, DescriptorClass)\
    namespace\
    {\
        MsgRegistration<MessageClass,DescriptorClass> MessageClass ##_factory_register;\
    } 

     


#endif
