/**
  * Timer.hpp
  * Here the different timer objects are implemented
  * Such objects are intended to be created by a block and activated by the timer-related
  * methods in the Block superclass (set_timer_at and set_periodic_timer) 
  * When a timer expires, it is returned to the block which scheduled it through its handle_timer method
  */


#ifndef _TIMER_HPP_
#define _TIMER_HPP_ 

#include <chrono>
#include <string>

#include <Block.hpp>

namespace bm { 

    /**
      * Timer virtual base class. Timers are always handled via this interface
      */
    class Timer
    {
    protected:
        std::chrono::system_clock::time_point m_time;
        Block& m_owner;     

        std::string m_name;
        unsigned int m_id;

    public:
        /**
          * Base class constructor
          * @param b reference to the block the timer is associated with
          * @param tp the absolute time point when the timer is supposed to expire
          * @param n the timer name (can be used by the block in order to distinguish among different timeout events)
          * @param id an (optional) block-scoped timer identifier (can be used by the block in order to distinguish among different timeout events)
          */

        Timer(Block& b, std::chrono::system_clock::time_point tp, std::string n, unsigned int id=0):
        m_time(tp),
        m_owner(b),
        m_name(n),
        m_id(id)
        {}

        virtual ~Timer()
        {}

        /**
          * This object is non copiable and non moveable
          */
        Timer(const Timer &) = delete;
        Timer& operator=(const Timer &) = delete;
        Timer(Timer &&) = delete;
        Timer& operator=( Timer &&) = delete;

        /**
          * @return the timer object name
          */
        std::string get_name() const
        {
            return m_name;
        }

        /**
          * resets the expiration time for this timer
          * @param tp the new expiration time point
         */ 
        void set_time_point( std::chrono::system_clock::time_point tp)
        {
            m_time=tp;
        }

        /**
          * @return the timer's expiration time point
          */
        std::chrono::system_clock::time_point 
        time_point() const
        {
            return m_time;
        }

        /**
          * @return the timer's block-scoped id
          */
        unsigned int get_id() const
        {
            return m_id;
        }

        /**
          * @return a reference to the block this timer is associated with
         */ 
        Block& owner() const
        {
            return m_owner;
        }

        /**
          * pure virtual method which has to be implemented by the derived class
          * It is called after the timer has expired in order to reschedule it
          * @return the new expiration time. If this equals the previous one the timer is not rescheduled.
         */ 
        virtual std::chrono::system_clock::time_point next_time() = 0;

    };

    /**
      * Derived class for one-shot timers.
      * These timers are only meant to be executed once
      */
    class OneShotTimer: public Timer
    {
    public:
        /**
          * Class constructor
          * @param b reference to the block the timer is associated with
          * @param tp the absolute time point when the timer is supposed to expire
          * @param n the timer name (can be used by the block in order to distinguish among different timeout events)
          * @param id an (optional) block-scoped timer identifier (can be used by the block in order to distinguish among different timeout events)
          */
        OneShotTimer (Block& b, std::chrono::system_clock::time_point tp, const std::string& n, unsigned int id=0)
        : Timer(b,  tp, n, id)
        {}

        virtual ~OneShotTimer()
        {}
        /**
          * Implementation of the base class virtual method
          * @return the same time point when the timer expired. This causes the timer not to be rescheduled.
          */
        std::chrono::system_clock::time_point 
        next_time()
        {
            return m_time;
        }

    };

    /**
      * Derived class for one-shot timers.
      * These timers are automatically rescheduled after expiration
      */
    class PeriodicTimer: public Timer
    {
    protected:
        std::chrono::microseconds m_period;

    public:

        /**
          * Class constructor
          * @param b reference to the block the timer is associated with
          * @param tp the absolute time point when the timer is supposed to expire for the first time
          * @param n the timer name (can be used by the block in order to distinguish among different timeout events)
          * @param id a block-scoped timer identifier (can be used by the block in order to distinguish among different timeout events)
          * @param p the time interval between two consecutive timer executions (in microseconds) 
          */
        PeriodicTimer (Block& b,  std::chrono::system_clock::time_point tp, const std::string& n, unsigned int id, std::chrono::microseconds p)
        : Timer(b,  tp, n, id), m_period(p)
        {}

        virtual ~PeriodicTimer()
        {}

    // FIXME check whether this method is needed    
    //    PeriodicTimer (const PeriodicTimer& t)
    //    : Timer(t.m_owner,t.m_time,t.m_name,t.m_id), m_period(t.m_period)
    //    {}

        /**
          * Implementation of the base class virtual method
          * @return the next time point when this timer will fire. This equals the last scheduled execution time point plus the period
          */
        std::chrono::system_clock::time_point 
        next_time()
        {
            return m_time=std::chrono::system_clock::now()+m_period;
        }
    };

} // namespace bm

#endif /* _TIMER_HPP_ */
