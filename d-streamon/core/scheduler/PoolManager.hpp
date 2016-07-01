#ifndef _POOLMANAGER_HPP_
#define _POOLMANAGER_HPP_ 

/**
  * PoolManager.hpp
  * Singleton directory of Blockmon's thread pools
  * It is used in order to dispatch active blocks to thread pools, according to the user specifications,
  * as well as to create, manage and delete the thread pools themselves.
  */



#include <ThreadPool.hpp>
#include <pugixml.hpp>

#include <map>
#include <memory>
#include <vector>
#include <cstdio>


using namespace pugi;

namespace bm
{
    /*
     * Forward declaration
     */
    class Block;

    
    class PoolManager
    {
        std::map<std::string,std::shared_ptr<ThreadPool> > m_map;
        /**
          * class constructor
          * private,as in Meyer's singleton
          */
        PoolManager()
        : m_map()
        {}

        ~PoolManager()
        {}
        
        /**
          * this object is non-moveable and non-copiable
          */
        PoolManager(const PoolManager &) = delete;
        PoolManager& operator=(const PoolManager &) = delete;
        PoolManager(PoolManager &&) = delete;
        PoolManager& operator=(PoolManager &&) = delete;


    public:
        /**
          * unique instance accessor as in Meyer's singleton
          * @return a reference to the only instance of this class
          */
        static	PoolManager& 
        instance()
        {
            static PoolManager pm;
            return pm;
        }
        /**
          * Adds a new thread pool to the directory
          * @ param n an xml subtree (parsed as a pugixml node) describing the thread pool, as it appears in the composition specification
         */ 
        void create_pool(xml_node n)
        {

            std::string name = n.attribute("id").value();
            int nthreads = n.attribute("num_threads").as_int();
            if((name.length() == 0)||(nthreads == 0))
                throw std::runtime_error("could not parse thread pool paramters");

            std::vector<unsigned int> affinity;
            for (xml_node t = n.child("core"); t; t = t.next_sibling("core"))
            {
                const char* num = t.attribute("number").value();
                if(!num)
                    throw std::runtime_error("cannot find core number");
                unsigned int corenum = 0xffffffff;
                sscanf(num,"%u",&corenum);
                if(corenum == 0xffffffff)
                {
                    throw std::runtime_error("cannot parse core number");
                }
                affinity.push_back(corenum);
            }
            auto it = m_map.find(name);
            if(it != m_map.end())
                throw std::runtime_error(name.append(" : thread pool already exists"));
            m_map[name] = std::make_shared<ThreadPool>(nthreads,affinity);
        }

        /**
          * dispatches a block to a given thread pool.
          * Notice that an active block as to be explicitly removed from its pool, otherwise it will not be destroyed
          * @param name the thread pool identifier
          * @param b a shared pointer to the block
          */
        void add_to_pool(const std::string &name, std::shared_ptr<Block> b)
        {
            auto it = m_map.find(name);
            if(it == m_map.end())
                throw std::runtime_error(std::string(name).append(" : thread pool does noe exist"));
            it->second->add_task(std::move(b));
        }
        /**
          * Removes a block from a given thread pool.
          * Notice that an active block as to be explicitly removed from its pool, otherwise it will not be destroyed
          * @param name the thread pool identifier
          * @param b a shared pointer to the block
          */
        void remove_from_pool(const std::string &name, const Block& b)
        {
            auto it = m_map.find(name);
            if(it == m_map.end())
                throw std::runtime_error(std::string(name).append(" : thread pool does noe exist"));
            it->second->remove_task(b);
        }

        /**
          * Removes a thread pool from the directory and destroys it.
          * Notice that any active blocks assigned to the pool will not be executed anymore.
          * @param name the thread pool identifier
          */
        void delete_pool(const std::string &name)
        {
            m_map.erase(name);
        }

        /**
          * Creates all of the threads of all of the thread pools.
          * This is when BlockMon actually starts working.
          */
        void start()
        {
            auto end = m_map.end();
            for(auto cur = m_map.begin(); cur!=end; ++cur)
            {
                cur->second->run();
            }

        }


        /**
          * Stops all of the threads of all of the thread pools.
          * This is needs to be done prior to any reconfiguration.
          */
        void stop()
        {
            auto end=m_map.end();
            for(auto cur=m_map.begin(); cur!=end; ++cur)
            {
                cur->second->stop();
            }
        }

    };

}


#endif /* _POOLMANAGER_HPP_ */
