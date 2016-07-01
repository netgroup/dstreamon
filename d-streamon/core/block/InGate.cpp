#include <InGate.hpp>
#include <Block.hpp>

namespace bm { 

    void InGate::receive(std::shared_ptr<const Msg>&& inm)
    {
        if(!m_owner->is_active()) {
            m_owner->receive_msg(std::move(inm),m_index);
        } else {
            m_queue->push(std::move(inm));
        }
    }

    void InGate::connect(OutGate& out)
    {
        m_peers.push_back(&out);
    }

    void InGate::disconnect(OutGate* in)
    {
        auto rm=std::remove(m_peers.begin(),m_peers.end(),in);

        auto end=m_peers.end();
        if(rm==end)
            throw std::runtime_error("trying to disconect a non-connected gate");

        m_peers.erase(rm,end);
    }

    void InGate::disconnect_all()
    {
        for (auto it=m_peers.begin(); it!=m_peers.end(); ++it)
        {
            (*it)->disconnect(this);
        }
        m_peers.clear();
        m_queue->reset();
    }


} // namespace bm






