#ifndef _OUTGATE_HPP_
#define _OUTGATE_HPP_ 

#include <cassert>
#include <utility>
#include <InGate.hpp>
#include<algorithm>

namespace bm { 

    class Block;


    class OutGate
    {
    public:
        OutGate(Block& own,int id)
        : m_owner(&own), m_peers(), m_index(id)
        {}

        ~OutGate()
        {}

        OutGate(const OutGate& other) = delete;
        OutGate& operator=(const OutGate& other) = delete;

        void swap(OutGate &other)
        {
            std::swap(m_owner, other.m_owner);
            std::swap(m_peers, other.m_peers);
            std::swap(m_index, other.m_index);
        }

        OutGate(OutGate &&other)
        : m_owner(std::move(other.m_owner)),
        m_peers(std::move(other.m_peers)),
        m_index(std::move(other.m_index))
        {} 

        OutGate& operator=(OutGate &&other)
        {
            OutGate tmp(std::move(other));
            tmp.swap(*this);
            return *this;
        }



        void connect(InGate& in)
        {
            m_peers.push_back(&in);
        }

        void disconnect(InGate* in)
        {
            auto rm=std::remove(m_peers.begin(),m_peers.end(),in);

            auto end=m_peers.end();
            if(rm==end)
                throw std::runtime_error("trying to disconect a non-connected gate");

            m_peers.erase(rm,end);


        }

        void disconnect_all()
        {

            for (auto it=m_peers.begin(); it!=m_peers.end(); ++it)
                (*it)->disconnect(this);
            m_peers.clear();
        }





        int deliver(std::shared_ptr<const Msg>&& m)
        {
            //std::cout<<"m peers = " << m_peers.size()<< "\n";
            assert(m_peers.size()>0);

            unsigned int e=m_peers.size();
            for(unsigned int i=0;i<e-1;++i)//first e-1
            {
                m_peers[i]->receive(std::shared_ptr<const Msg> (m));
            }
            m_peers[e-1]->receive(std::move(m));
            return 0;
        }

    private:
        Block* m_owner;
        std::vector<InGate*> m_peers;
        int m_index;

    };

} // namespace bm
#endif /* _OUTGATE_HPP_ */
