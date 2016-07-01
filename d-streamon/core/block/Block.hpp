/**
 * @file 
 *
 * Base class for BlockMon Blocks. A block is a small unit of processing,
 * that comm
 *
 * @section HOW TO BUILD A BLOCK
 *
 * 1. create a subclass of Block ( class MyBlock : public Block { ... }; )
 *    to make it easier on other developers, put this in the blocks/ 
 *    directory, and name it SomethingBlock, unless it meets one of the
 *    following two criteria:
 *    (i)  Messages sources (e.g. IPFIX importers, packet capture blocks)
 *         should be named SomethingSource.
 *    (ii) Message sinks for export (e.g., IPFIX exporters) should be
 *         named SomethingExporter.
 * 2. implement your constructor in terms of the Block constructor. Your
 *    block constructor should take two parameters, std::string name and 
 *    bool active, which will be supplied by BlockFactory. Resource-intensive
 *    initialization can be deferred to _configure(), below.
 * 3. implement the _configure() method to handle block configuration. 
 *    This receives a pugixml node representing the <params> element 
 *    containing block parameters. All blocks must implement _configure(),
 *    which in addition to configuration should also do any necessary
 *    creation of periodic timers or other resource-intensive initialization.
 * 4. implement the _receive_msg() method to receive messages (which you'll
 *    need to do unless your block is a Source).
 * 5. If your block uses timers (see Timer.hpp), implement the _handle_timer() 
 *    method, which gets called when the timers fire.
 * 6. If your block is runtime-reconfigurable, implement the 
 *    _update_config() method.
 * 7. If your block can be passively scheduled and is thread-safe, implement
 *    the _synchronize_access() method to return false. _synchronize_access()
 *    is called after _configure(), so your block's thread-safety can be 
 *    configuration dependent.
 * 8. If your block can only be actively scheduled, and requires
 *    high frequency periodic events without timing constraings (e.g., as
 *    a Source block busywaiting on packets, implement _do_async_processing().
 *    This method should do the smallest amount of processing that is
 *    practical, in order to retain scheduling flexibility.
 * 9. Use the REGISTER_BLOCK(classname, configname) macro at the end of your 
 *    SomethingBlock.cpp file macro to register your Block with the factory.
 *
 * Block scheduling
 * =============================
 * Blockmon supports active and passive block invocation. Active blocks are
 * called by Blockmon’s scheduler, while passive blocks are activated directly
 * within the same thread as the block which sends them a message. Blocks can
 * also be scheduled to run on a timer.
 *
 * Copyright notice goes here.
 */

#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_ 

#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <sstream>
#include <stdexcept>

#if __GNUC__ == 4 &&  __GNUC_MINOR__ == 4  
#include <cstdatomic>
#else 
#include <atomic>
#endif

#include <pugixml.hpp>


#include <InGate.hpp>
#include <OutGate.hpp>
#include <MpMcQueue.hpp>
#include <BlockVariable.hpp>

using namespace pugi;

namespace bm { 

    class Timer;

    /* maximum number of messages to dequeue at once in active mode */
    static const int MAX_MSG_DEQUEUE = 10;

    class Block {

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Class boilerplate: constructors, destructors, copyability
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    protected:

/**
 * Create a new Block with a given name and type. 
 *
 * @param name the name of the block
 * @param active TRUE if the block is actively scheduled, FALSE if passive.
  */

        Block(const std::string &name, bool active)
        : m_name(name),
          m_mutex(),
          m_issynchronized(true),
          m_isactive(active),
          m_running(false),
          m_input_ids(),
          m_output_ids(),
          m_ingates(),
          m_outgates(),
          m_last_in_id(0),
          m_last_out_id(0),
          m_timer_queue(),
          m_variables()
        {}
    public:
        /** Virtual destructor */
        virtual ~Block();
    protected:

        /** Null copy constructor for Block; Blocks cannot be copied. */
        Block(const Block &) = delete;

        /** Null copy assignment for Block; Blocks cannot be copied. */
        Block& operator=(const Block &) = delete;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Basic accessors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    public:

/**
 * Return the active status of this block.
 *
 * @return TRUE if the block is actively scheduled, FALSE if passive.
 */
        bool is_active() const
        {
            return m_isactive;
        }

/**
 * Set the active scheduling status of the block
 *
 * @param active TRUE if the block is actively scheduled, 
 *               FALSE if passive.
 */
        void set_active(bool active)
        {
            if (m_running) {
                throw std::runtime_error("cannot change execution mode while block is running");
            }
            
            m_isactive=active;
        } 

/**
 * Return the running status of this block. 
 *
 * NOTE: This facility is not yet used by BlockMon, and is included
 * for future reconfiguration support.
 *
 * @return TRUE if the block is presently running (that is, may 
 *         have its receive_msg() and handle_timer() methods called).
 */
        bool is_running() const
        {
            return m_running;
        }

/**
 * Set the running status of this block. Called by the
 * BlockMon infrastructure at thread startup and shutdown time.
 *
 * NOTE: This facility is not yet used by BlockMon, and is included
 * for future reconfiguration support.
 *
 * @param running TRUE if the block is presently running (that is, may 
 *         have its receive_msg() and handle_timer() methods called).
 */
        void set_running(bool running)
        {
            m_running = running;
        }

/**
 * Return the block's name
 *
 * @return the name of the block
 */
        const std::string& get_name() const
        {
            return m_name;
        }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Gate accessors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Get an input gate on this block, given a name.
 * Throws unless the named input gate exists.
 *
 * @param name the name of the input gate to get
 * @return the named gate by reference.
 */
        InGate& get_input_gate(const std::string& name)
        {
            std::map<std::string, int>::iterator it;
            it = m_input_ids.find(name);
            if (it == m_input_ids.end()) {
                throw std::runtime_error(std::string(name).append(": no such gate"));
            }
            return m_ingates[it->second];
        }
/**
 * Get an output gate on this block, given a name.
 * Throws unless the named output gate exists.
 *
 * @param name the name of the output gate to get
 * @return the named gate by reference.
 */
        OutGate& get_output_gate(const std::string& name) 
        {
            std::map<std::string, int>::const_iterator it;
            it = m_output_ids.find(name);
            if (it == m_output_ids.end()) 
                throw std::runtime_error(std::string(name).append(": no such gate"));
            return m_outgates[it->second];
        }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Entry points for the BlockMon infrastructure
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Entry point for configuration. Called by the BlockMon 
 * infrastructure at initial configuration time. 
 * Not overridable; derived classes must
 * implement the virtual method _configure() to handle configuration.
 *
 * @param xmlnode the <params> XML node containing block parameters
 */
        void configure(const xml_node& params_node)
        {
            _configure(params_node);
            m_issynchronized = _synchronize_access();
        }

/**
 * Entry point for reconfiguration. Called by the BlockMon 
 * infrastructure at reconfiguration time. Not overridable; derived 
 * classes must implement the virtual method _update_config() to 
 * handle reconfiguration. 
 *
 * @param xmlnode the <params> XML node containing block parameters
 * @return TRUE if the configuration is updateable, FALSE otherwise.
 */
        bool update_config(const xml_node& params_node)
        {
            if (_update_config(params_node)) {
                m_issynchronized = _synchronize_access();
                return true;
            } else {
                return false;
            } 
        }

/**
 * Entry point for passive message processing. Called by the BlockMon 
 * infrastructure when a message is sent to this Block.
 * Handles synchronization for thread-unsafe _receive_msg implementations. 
 * Not overridable; derived classes must implement the virtual method 
 * _receive_msg() to receive messages.
 *
 * @param m a shared pointer to the received message
 * @param gate_id the id of the gate on which the message was received
 */
        void receive_msg(std::shared_ptr<const Msg>&& m, int gate_id)
        {
            if (m_issynchronized) {
                std::lock_guard<std::mutex> receive_guard(m_mutex);
                _receive_msg(std::move(m), gate_id);
            } else {
                _receive_msg(std::move(m), gate_id);
            }
        }

	

/**
 * Entry point for timer processing. Called by the BlockMon 
 * infrastructure when a timer event occurs for this Block.
 * Handles queueing for actively scheduled blocks and synchronization 
 * for thread-unsafe _handle_timer implementations. Not overridable; 
 * derived classes must implement the virtual method _handle_timer()
 * to handle timers.
 *
 * @param t a shared pointer to the timer on which the event occurred
 */
        void handle_timer(std::shared_ptr<Timer> t)
        {
            if (m_isactive)
            {
                m_timer_queue.push(std::move(t));
            }
            else if (m_issynchronized)
            {
                std::lock_guard<std::mutex> tmp(m_mutex);
                _handle_timer(std::move(t));
            }
            else
            {
                _handle_timer(std::move(t));
            }
        }

 /**
  * Entry point for the active runloop. Called by the BlockMon infrastructure
  * within a thread in the assigned threadpool. 
  *
  * Handles queued timers and messages via _handle_timer() and _receive_msg(), 
  * and asynchronous processing via _do_async_processing().
  */
        void run();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Exposed variable accessors
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * this method displays the block variable exposed by this block and specifies their access permissions (read/write) 
 * @return a string reporting a human readable list of variable with their names and permissions
 */
        std::string list_variables();

/**
 * this method allows to read the value of a block variable. In order for this
 * to happen, such variable has to be registered as readable
 * @return in case read succeeded a string reporting the human readable value of the variable, otherwise an empty string 
 */
        virtual std::string read_variable(const std::string& v_name)
        {
            auto it = m_variables.find(v_name);
            if (it == m_variables.end())
            {
                throw std::runtime_error(std::string(v_name).append(" :no such variable"));
            }
            return ((it->second)->read());
        }

/**
 * writes a value (expressed as a string) to a block variable.
 * Notice that the interface is type agnostic: the streaming operators in the will convert the content into a type-specific value. 
 * For example if the control plane  needs to write 123 into an integer type variable, the content of the string s will be the human-readable "123"
 * which will be converted into its numeric value by the streaming operator.
 * @param v_name name of the variable
 * @param val value to be written into the variable
 */
        virtual void write_variable(const std::string& v_name, const std::string& val)
        {
            auto it = m_variables.find(v_name);
            if(it==m_variables.end())
            {
                throw std::runtime_error(std::string(v_name).append(" :no such variable"));
            }
            (it->second)->write(val);
        }

    /* only subclasses can register variables */
    protected:

/** 
 * Register a variable which can be externally shared outside the block.
 * See BlockVariable.hpp for documentation on creating and using exposed variables.
 * 
 * @param name name by which the variable can be accessed
 * @param v a shared pointer to the variable to register (creating it with the helper functions in BlockVariable.hpp is recommended)
 */
        void register_variable(const std::string& name, std::shared_ptr<BlockVariable>&& v)
        {
            auto it = m_variables.find(name);
            if(it != m_variables.end()) {
                throw std::runtime_error(std::string(name).append(" variable already registered"));
            }
            m_variables[name] = std::move(v);
        }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Gate accessors for use by derived classes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    protected:
 
 /**
  * Send a message on a given gate. If connected to a passive block, the
  * downstream receive_msg will be invoked directly; otherwise the message
  * will be queued.
  *
  * @param m the message to send
  * @param gate_id the id of the gate to send the message on, 
  *                as returned by register_output_gate
  */
        void send_out_through(std::shared_ptr<const Msg>&& m, int id)
        {
            assert((id >= 0) && (id < (int)m_outgates.size()));
            m_outgates[id].deliver(std::move(m));
        }

 /**
  * Register an input gate
  * 
  * @param name the name of the gate to register
  * @return the id of the registered gate
  */
        int register_input_gate(const std::string &name);

 /**
  * Register an output gate
  * 
  * @param name the name of the gate to register
  * @return the id of the registered gate
  */
        int register_output_gate(const std::string &name);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Timer accessors for use by derived classes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * Set a one-shot timer to fire on this block at a specified point in time.
 *
 * @param ts the time at which to fire the timer
 * @param name the name of the timer
 * @param id the id of the timer; retrievable with Timer::get_id() for later
 *           quick comparison
 */
        void set_timer_at(std::chrono::system_clock::time_point ts, 
                          const std::string &name, 
                          unsigned int id);

/**
 * Set a periodic timer to fire on this block with a specified period
 *
 * @param us the period of the timer in microseconds.
 * @param name the name of the timer
 * @param id the id of the timer; retrievable with Timer::get_id() for later
 *           quick comparison
 */
        void set_periodic_timer(std::chrono::microseconds us, 
                                const std::string &name, 
                                unsigned int id);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Logging to be used by derived classes.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
        enum log_type {log_debug, log_info, log_warning, log_error};

/**
 * Print a log message.
 * 
 * FIXME make this not suck
 * 
 * @param l the log message to print
 * @param the type of the log message (one of debug, info, warning, error)
 */
        void blocklog(std::string l, log_type t)
        {
            std::cerr << m_name << '\t' << l << t << std::endl;
        }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Entry points to be overriden by derived classes.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    protected:
        
/**
 * Configure the block given an XML element containing configuration.
 * Called before the block will begin receiving messages.
 *
 * MUST be overridden in a derived class. 
 * Configuration errors should be signaled by throwing.
 *
 * @param xmlnode the <params> XML element containing block parameters
 */
        virtual void _configure(const xml_node& xmlnode) = 0;
 
/**
 * Update the configuration on a running block given an XML 
 * element containing configuration. Can veto a reconfiguration by
 * returning false, after which the block will be destroyed, recreated,
 * and configured from scratch.
 *
 * The default implementation simply returns false, requiring a restart.
 *
 * @param xmlnode the <params> XML node containing block parameters.
 * @return true if update was successful, false if impossible.
 */
        virtual bool _update_config(const xml_node& xmlnode) { return false; }

/**
 * Handle a message sent from another Block.
 *
 * Should be overridden by any derived class that can receive messages;
 * default does nothing. Errors should be logged. Fatal
 * errors should be handled by throwing.
 *
 * @param m a shared pointer to the message to handle
 * @param gate_id the gate ID of the input gate on which the message was 
 *                received, as returned by register_input_gate()
 *
 */
        virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int gate_id) {}

/**
 * Handle a timer event.
 *
 * Should be overridden by any derived class that uses timers;
 * default throws a logic error. Errors should be logged. Fatal
 * errors should be handled by throwing. 
 *
 * @param t the timer on which the event occurred.
 *
 */
        virtual void _handle_timer(std::shared_ptr<Timer>&& t) {
            throw std::logic_error("timer set on block without _handle_timer()");
        }
            
        
/**
 * Do asynchronous processing as part of an active Block's runloop.
 * Not called for passively scheduled Blocks. The default implementation
 * does nothing. This method is meant for special, high-frequency periodic
 * events without timing requirements, and should be used carefully.
 */
        virtual void _do_async_processing() { }

/**
 * Determine whether the block's _receive_msg() and _handle_timer() methods
 * should be synchronized (i.e., should allow only one thread at a time to
 * execute them. Called after configuration or reconfiguration time.
 *
 * Derived classes SHOULD override this method to return false if they are 
 * threadsafe; the default implementation always returns true.
 *
 * @return the synchronized status of the methods in the block
 */
        virtual bool _synchronize_access() { return true; }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Member variables
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    protected:
        const std::string m_name;

    private:
        std::mutex m_mutex;
        bool m_issynchronized;
        bool m_isactive;	
        bool m_running;
        
        // Input and output gates
        std::map<std::string, int > m_input_ids;
        std::map<std::string, int > m_output_ids;
        std::vector<InGate> m_ingates;
        std::vector<OutGate> m_outgates;
        std::atomic_int m_last_in_id;
        std::atomic_int m_last_out_id;

        MpMcQueue<Timer> m_timer_queue;

        std::map<std::string, std::shared_ptr<BlockVariable> > m_variables;
    };

} // namespace bm

#endif /* _BLOCK_HPP_ */
