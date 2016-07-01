/**
 *MemoryBatch.hpp
 *Basic class and methods for handling batch allocations
 *This should only work with messages providing a memory_not_owned constructor (e.g. RawPacket)
 *  
 * Copyright notice goes here.
 */

/* Idem hack */
#ifndef _MEMORY_BLOCK_HPP_
#define _MEMORY_BLOCK_HPP_ 

#include <iostream>
#include <Msg.hpp>


/**
 *class MemoryBatch
 *an object of this class allocates a big buffer where messages can be created in-place through the alloc_msg_from_buffer function
 * notice that an object of this class should never be statically allocated. On the contrary it MUST be dynamically allocated and owned by a shared_ptr
 * when the buffer is full, the alloc_msg_from_buffer function releases its ownership and automatically allocates a new object
 * messages within the buffer are handled through shared pointers (the c++0x alias constructor semantic is used)
 * whenever the last pointer to a message in the buffer is released, the whole buffer is automatically freed
 * for an example of the usage of this mechanism refer to Sniffer.cpp

 */

namespace bm { 

    class MemoryBatch
    { 
    public:
        /**
         * class constructor
         * @param size size of the big buffer to be allocated
         */

        MemoryBatch(size_t size)
        : m_buff(new char[size]), m_size(size), m_curr(0)
        {}

        /**
          * this is the only function which is allowed to work with this object
         */
        template <typename T, typename ...Ti>
        friend
        std::shared_ptr<const Msg> 
        alloc_msg_from_buffer(std::shared_ptr<MemoryBatch>& mem_block, size_t additional_space, Ti && ...arg);

        /**
         * class destructor: actually deallocates the buffer
         * it is only call when the shared pointers to all of the internal messages have been released
         */

        ~MemoryBatch()
        {
            delete []m_buff;
        }

        /**
          this object is not copyable nor movable*/
          
        MemoryBatch(const MemoryBatch &) = delete;
        
        MemoryBatch & operator=(const MemoryBatch &) = delete;
        
        MemoryBatch( MemoryBatch &&) = delete;
        MemoryBatch & operator=( MemoryBatch &&) = delete;

    private:
        char *
        addr()
        {
            return m_buff;
        }

        const char *
        addr() const
        {
            return m_buff;
        }

        char *
        reserve(size_t size)
        {
            size_t curr = m_curr;
            if (curr+size > m_size) {
                return NULL;
            }
            m_curr+=size;
            return m_buff + curr; 
        }

        char * m_buff;
        size_t m_size;
        size_t m_curr;

    };

    /**
    *  this functions builds a message within the big buffer and returns a shared pointer to it
    *  @param T the message type
    *  @param Ti all of the arguments to the specific class constructor. 
    *  @param mem_block the shared pointer to the MemoryBatch object which will store the message
    *  @param additional_space the additional buffer the message needs (besides the class instance itself). This may the size of the packet buffer, as in Packet
    */  
    template <typename T, typename ...Ti>
    std::shared_ptr<const Msg> 
    alloc_msg_from_buffer(std::shared_ptr<MemoryBatch>& mem_block, size_t additional_space, Ti && ...arg)
    {
        char * mbuf = mem_block->reserve(sizeof(T) + additional_space);
        if(!mbuf)
        {
            mem_block.reset();
            mem_block = std::make_shared<MemoryBatch>(4096*256);
            mbuf = mem_block->reserve(additional_space + sizeof(T)); 
        }
        new (mbuf) T(bm::memory_not_owned, sizeof(T), std::forward<Ti>(arg)...);
        return std::shared_ptr<const Msg>(mem_block,reinterpret_cast<const Msg*>(mbuf)); 
    }

} // namespace bm


#endif /* _MEMORY_BLOCK_HPP_ */
