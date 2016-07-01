#ifndef _INGATE_HPP_
#define _INGATE_HPP_ 


#include<Msg.hpp>
#include <vector>
#include<algorithm>
#include<DBQueue.hpp>

namespace bm { 

    class OutGate;
    class Block;
    
    class InGate
    {
    public:
        typedef DBQueue<std::shared_ptr<const Msg>,1024> queue_type;


        InGate(Block& own,int id)
        : m_owner(&own), m_queue(new queue_type), m_peers(), m_index(id)
        {}

        ~InGate()
        {}

        InGate(const InGate&) = delete;
        InGate& operator=(const InGate&) = delete;

        void swap(InGate &other)
        {
            std::swap(m_owner, other.m_owner);
            std::swap(m_queue, other.m_queue);
            std::swap(m_peers, other.m_peers);
            std::swap(m_index, other.m_index);
        }

        InGate(InGate&& other)
        : m_owner(std::move(other.m_owner)),
          m_queue(std::move(other.m_queue)), 
          m_peers(std::move(other.m_peers)), 
          m_index(std::move(other.m_index))
        {}

        InGate& operator=(InGate&& other)
        { 
            InGate tmp(std::move(other));
            tmp.swap(*this);
            return *this;
        }

        void connect(OutGate& out);

        std::shared_ptr<const Msg> dequeue()
        {
            std::shared_ptr<const Msg> tmp;
            m_queue->pop(tmp);
            return tmp;
        }

        void disconnect(OutGate* in);

        void receive(std::shared_ptr<const Msg>&& inm);
        void disconnect_all();

    private:
        Block *m_owner;
        std::unique_ptr<queue_type> m_queue;
        std::vector<OutGate*> m_peers;
        int m_index;
    };

} // namespace bm

#endif /* _INGATE_HPP_ */
