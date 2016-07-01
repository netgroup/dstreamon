/**
 *ClassId.hpp
 *this mechanism allows to automatically generate message codes
 *it is based on hash portable hashing of the demangled class name
 *collision among codes (that should be highly unlikely) are detected at run time
 */





#ifndef __CLASSID_H__
#define __CLASSID_H__


#include <cxxabi.h>
#include <typeinfo>
#include <memory>
#include <stdexcept>
#include <string>
#include<set>
#include <mutex>                

namespace detail
{

/** 
 * only needed to allow template specializations
 */
template<typename T>
struct nullstruct {};


/**
 * this class takes note of the code assigned to messagea: it is used upon code assignment in order to check that collisions did not happen
 * the user should not use this, but only the MSG_ID macro or the type_to_id<T>::id() function 
 */

class IdDirectory
{
    std::set<int> m_assigned_ids;
    std::mutex m_mutex;

    IdDirectory():m_assigned_ids(), m_mutex()
    {}


    static unsigned int hash(const char* b,size_t s)
    {
        int remaining_length=s;
        unsigned int hash=0x66B566B5;
        const char* it = b;
        while(remaining_length >= 2)
        {
            hash ^=    (hash <<  7) ^  (*it++) * (hash >> 3);
            hash ^= (~((hash << 11) + ((*it++) ^ (hash >> 5))));
            remaining_length -= 2;
        }
        if (remaining_length)
        {
            hash ^= (hash <<  7) ^ (*it) * (hash >> 3);
        }
        return hash;
    }






public:

    /**
     * Meyers' idiom for singleton classes
    */

    static IdDirectory& instance()
    {

        static IdDirectory the_instance;
        return the_instance;
    }

    /**
      * actual code generation function: T is the message class
      */

    template<typename T> 
    static int compute_id()
    {
        std::string name(typeid(nullstruct<T>()).name());
        int status;
        std::shared_ptr<char> ret(abi::__cxa_demangle(name.c_str(),0,0, &status), ::free);
        if(status<0)
            throw std::runtime_error(__PRETTY_FUNCTION__);
        std::string demangled (ret.get());
        int ret_id=(static_cast<int>(hash(demangled.c_str(),demangled.length())));
        std::lock_guard<std::mutex> protect_set (instance().m_mutex); //protect access to the shared set
        if(instance().m_assigned_ids.find(ret_id)!=instance().m_assigned_ids.end())
            throw std::runtime_error("BAAAAAAAAAAAAD: collision among message ids");
        instance().m_assigned_ids.insert(ret_id);
        return ret_id;

    }


};


}//namespace detail


/**
  * template class for instantiating the message id
  * T is the message class
  */

template<typename T>
struct type_to_id
{
    static int id()
    {
        static int id=detail::IdDirectory::instance().compute_id<T>();
        return id;
    }
};


/**
  * commodity macro for instantiating the message id
  * T is the message class
  */
#define MSG_ID(msgtype)\
type_to_id<msgtype>::id()


#endif
