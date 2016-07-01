#include<CompositionManager.hpp>
#include<PoolManager.hpp>
#include<TimerThread.hpp>


#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <pugixml.hpp>

using namespace pugi;
using namespace bm;

namespace
{

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Timer thread handline 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
  std::thread* tt=NULL;

  void start_timer_thread() {
#ifdef C_DEBUG
    std::cout << "c++: start_timer_thread" << std::endl;
#endif
    tt = new std::thread(std::ref(TimerThread::instance()));
  }
	
  void stop_timer_thread() {
#ifdef C_DEBUG
    std::cout << "c++: stop_timer_thread" << std::endl;
#endif
    TimerThread::instance().stop();
    tt->join();
    delete tt;
  }
   
  void add_composition(std::string filename)
  {
#ifdef C_DEBUG
    std::cout << "c++: add_composition "<< filename << std::endl;
#endif
    CompositionManager::instance().add_composition(filename);
  }

  void delete_composition(std::string comp_id)
  {
#ifdef C_DEBUG
    std::cout << "c++: delete_composition " << comp_id << std::endl;
#endif
    CompositionManager::instance().delete_composition(comp_id);
  }
   
  void start_schedulers()
  {
#ifdef C_DEBUG
    std::cout << "c++: start_schedulers" << std::endl;
#endif
    PoolManager::instance().start();
  }

  void stop_schedulers()
  {
#ifdef C_DEBUG
    std::cout << "c++: stop_schedulers" << std::endl;
#endif
    PoolManager::instance().stop();
  }

  void create_connection(const std::string& from_composition,\
                         const std::string& from_block,      \
                         const std::string& from_gate,       \
                         const std::string& to_composition,  \
                         const std::string& to_block,        \
                         const std::string& to_gate)
  {
#ifdef C_DEBUG
  std::cout << "c++: create+connection" \
            << from_composition \
            <<from_block \
            <<from_gate \
            <<to_composition \
            <<to_block \
            <<to_gate \
            <<std::endl;
#endif
  OutGate& src=CompositionManager::instance().get_composition(from_composition).get_block(from_block)->get_output_gate(from_gate);
  InGate& dst=CompositionManager::instance().get_composition(to_composition).get_block(to_block)->get_input_gate(to_gate);
  src.connect(dst);
  dst.connect(src);
  }

  void delete_connection(const std::string& from_composition,\
                         const std::string& from_block,      \
                         const std::string& from_gate,  \
                         const std::string& to_composition, \
                         const std::string& to_block,       \
                         const std::string& to_gate)
  {  
#ifdef C_DEBUG
  std::cout << "c++: delete_connection"\
            << from_composition\
            << from_block\
            << from_gate\
            << to_composition\
            << to_block\
            << to_gate\
            << std::endl;
#endif
  OutGate& src=CompositionManager::instance().get_composition(from_composition).get_block(from_block)->get_output_gate(from_gate);
  InGate& dst=CompositionManager::instance().get_composition(to_composition).get_block(to_block)->get_input_gate(to_gate);
  src.disconnect(&dst);
  dst.disconnect(&src);
  }

  void create_block(const std::string& comp_id,\
                    const std::string& name,\
                    const std::string& type,\
                    bool active,\
                    const std::string& config)
  {  
#ifdef C_DEBUG
    std::cout << "c++: create_block" << comp_id << name << type << std::endl;
#endif
    CompositionManager::instance().get_composition(comp_id).create_block(name, type, active, config);
  }

  void delete_block(const std::string& comp_id, const std::string& name)
  {  
#ifdef C_DEBUG
    std::cout << "c++: delete_block" << comp_id << name <<std::endl;
#endif
    CompositionManager::instance().get_composition(comp_id).delete_block(name);
  }

  int update_block(const std::string& comp_id,\
                   const std::string& name,\
                   bool active,\
                   const std::string& config)
  {
#ifdef C_DEBUG
    std::cout << "c++: update_block" << comp_id << name << std::endl;
#endif
    xml_document config_info;
    if(!config_info.load(config.c_str()))
      throw std::runtime_error("update_config::cannot parse config info");
    xml_node confxml = config_info.child("params");
    if(!confxml)
      throw std::runtime_error("Configuration: cannot find params node");
    CompositionManager::instance().get_composition(comp_id).get_block(name)->set_active(active);
    return CompositionManager::instance().get_composition(comp_id).get_block(name)->update_config(confxml);
  }
   
  void add_block_to_thread_pool(const std::string& comp_id,\
                                const std::string& name,\
                                const std::string& pool)
  {
#ifdef C_DEBUG
    std::cout<<"c++: add_block_to_thread_pool" << comp_id << name << std::endl;
#endif
    std::shared_ptr<Block> b= CompositionManager::instance().get_composition(comp_id).get_block(name);
    PoolManager::instance().add_to_pool(pool,b);
  }

  void remove_block_from_thread_pool(const std::string& comp_id,\
                                     const std::string& name,\
                                     const std::string& pool)
  {
#ifdef C_DEBUG
    std::cout << "c++: remove_block_from_thread_pool" << comp_id << name << std::endl;
#endif
    std::shared_ptr<Block> b = CompositionManager::instance().get_composition(comp_id).get_block(name);
    PoolManager::instance().remove_from_pool(pool, *b);
  }

  void add_thread_pool(const std::string& pool_config)
  {                   
#ifdef C_DEBUG
    std::cout << "c++: add thread pool" << pool_config << std::endl;
#endif

    xml_document config_info;
    if(!config_info.load(pool_config.c_str()))
       throw std::runtime_error("add_thread_pool::cannot parse config info");
    xml_node poolxml = config_info.child("threadpool");
    if(!poolxml)
       throw std::runtime_error("Configuration: cannot find block node");
    PoolManager::instance().create_pool(poolxml);  
  }

   void remove_thread_pool(const std::string& poolname)
   {  
#ifdef C_DEBUG
     std::cout << "c++: remove thread pool" << poolname << std::endl;
#endif
     PoolManager::instance().delete_pool(poolname);
   }
	
   std::string read_block_variable(const std::string& comp_id,\
                                   const std::string& name,\
                                   const std::string& variable)
  {
      return CompositionManager::instance().get_composition(comp_id).get_block(name)->read_variable(variable);
  }

  void write_block_variable(const std::string& comp_id,\
                           const std::string& name,\
                           const std::string& variable,\
                           const std::string& val)
  {  
    CompositionManager::instance().get_composition(comp_id).get_block(name)->write_variable(variable, val);
  }

  std::string list_variables(const std::string & comp_id, const std::string& name)
  {
    return CompositionManager::instance().get_composition(comp_id).get_block(name)->list_variables();
  }
}


BOOST_PYTHON_MODULE(blockmon)
{     
    using namespace boost::python;

    // scheduler functions
    def("stop_timer", stop_timer_thread);
    def("start_timer", start_timer_thread);
    def("start_schedulers", start_schedulers);
    def("stop_schedulers", stop_schedulers);

    // composition functions
    def("add_composition", add_composition, arg("filename") );
    def("delete_composition", delete_composition, arg("filename") );

    // connection functions
    def("create_connection", create_connection, \
        (arg("from_composition"),\
         arg("from_block"),\
         arg("from_gate"),\
         arg("to_composition"),\
         arg("to_block"),\
         arg("to_gate")));
    def("delete_connection", delete_connection,\
        (arg("from_composition"),\
         arg("from_block"),\
         arg("from_gate"),\
         arg("to_composition"),\
         arg("to_block"),\
         arg("to_gate")));

    // block functions
    def("create_block", create_block,\
        (arg("composition"),\
         arg("name"),\
         arg("type"),\
         arg("active"),\
         arg("config")));
    def("update_block", update_block,\
        (arg("composition"),\
         arg("name"),\
         arg("active"),\
         arg("config")));
    def("delete_block", delete_block,\
        (arg("composition"),\
         arg("name")));

    // thread functions
    def("add_thread_pool", add_thread_pool, arg("pool_config"));
    def("remove_thread_pool",remove_thread_pool, arg("poolname"));
    def("add_block_to_thread_pool", add_block_to_thread_pool,\
        (arg("composition"),\
         arg("name"),\
         arg("pool")));
    def("remove_block_from_thread_pool", remove_block_from_thread_pool,\
        (arg("composition"),\
         arg("name"),\
         arg("pool")));

    // variables functions
    def("write_block_variable", write_block_variable,\
        (arg("composition"), \
         arg("name"),\
         arg("variable"),\
         arg("val")));
    def("read_block_variable", read_block_variable,\
        (arg("composition"),\
         arg("name"),\
         arg("variable")));
    def("list_variables", list_variables,\
        (arg("composition"),\
         arg("name")));
}
