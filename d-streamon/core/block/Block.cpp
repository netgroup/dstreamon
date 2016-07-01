#include <Block.hpp>
#include <TimerThread.hpp>

namespace bm { 

    void Block::run()
    {
        int nb = m_ingates.size();

        while (1)
        {
            std::shared_ptr<Timer> t(std::move(m_timer_queue.pop()));
            if(!t) break;
            _handle_timer(std::move(t));
        }

        for (int i=0; i<nb; ++i)
        {
            for (int n=0; n<MAX_MSG_DEQUEUE; ++n)
            {
                std::shared_ptr<const Msg> m(std::move(m_ingates[i].dequeue()));
                if (!m) break;
                _receive_msg(std::move(m),i);
            }
        }
        _do_async_processing();
    }

    int Block::register_input_gate(const std::string& name) //base class registers gates
    {
        int id = m_last_in_id++;
        m_ingates.push_back(InGate(*this,id)); 
        m_input_ids.insert(std::pair<std::string, int>(name,id));
        return id;
    }

    int Block::register_output_gate(const std::string& name)
    {
        int id = m_last_out_id++;
        m_outgates.push_back(OutGate(*this,id)); 
        m_output_ids.insert(std::pair<std::string, int>(name,id));
        return id;
    }

    std::string Block::list_variables()
    {
        std::stringstream ss(std::stringstream::in | std::stringstream::out);
        for (auto it = m_variables.begin(); it != m_variables.end(); ++it)
        {
            ss << it->first;

            if((it->second)->is_readable())
                ss << ",read"; 
            if((it->second)->is_writable())
                ss << ",write";
            ss << ";";
        }
        
        std::string ret;
        ss >> ret;
        return ret;
    }


    Block::~Block(void)
    {
         TimerThread::instance().remove_references(*this);
    }
    
    void Block::set_timer_at(std::chrono::system_clock::time_point ts, const std::string &name, unsigned int id)
    {
        std::shared_ptr<Timer> t(static_cast<Timer*>(new OneShotTimer(*this, ts, name, id)));
        TimerThread::instance().schedule_timer(std::move(t));
    }

    void Block::set_periodic_timer(std::chrono::microseconds us, const std::string &name, unsigned int id)
    {
        std::shared_ptr<Timer> t(new PeriodicTimer(*this, std::chrono::system_clock::now() + us, name, id, us));
        TimerThread::instance().schedule_timer(std::move(t));
    }

} // namespace bm
