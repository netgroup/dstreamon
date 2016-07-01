#ifndef __COMPOSITION_HH__
#define __COMPOSITION_HH__

/**
  * Composition.hpp
  * Object handling the blocks and links associated to a composition
  * It holds an internal directory of the blocks and handles installation and deletion
  * Part of the API is called from the python bindings, while other methods are intended for support of the legacy c++-only implementation
 */ 

#include <stdexcept>
#include <string>
#include <map>
#include <pugixml.hpp>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <Block.hpp>
#include <BlockFactory.hpp>


using namespace pugi;

namespace bm
{
    class Composition
    {
        std::map< std::string, std::shared_ptr<Block> > m_map;
        const std::string m_composition_id;
     
        /**
          * composition is not moveable nor copiable
          */

        Composition(const Composition &)=delete;
        Composition & operator=(const Composition &)=delete;
        Composition(Composition &&)=delete;
        Composition & operator=(Composition &&)=delete;

        /**
          * Adds a block to the composition and configures it
          * This is done before the block is connected to the others
          * This is the internal implementation, which is called both by different public methods
          * @param type the block type
          * @param name the name of the block instance
          * @param active boolean flag defining whether the block is active or passive
          * @param params the xml block-specific configuration subtree 
         */
        void create_block_from_parsed(const std::string& type, const std::string& name, 
                           bool active, const xml_node& params);

        /**
          * Helper function, retrieve a reference to a given output gate, throws upon failure
          * @param from_block block name
          * @param from_gate gate name
          */
        OutGate& find_output_gate( const std::string& from_block,const std::string& from_gate);


        /**
          * Helper function, retrieve a reference to a given output gate, throws upon failure
          * @param from_block block name
          * @param from_gate gate name
          */
        InGate& find_input_gate( const std::string& to_block,const std::string& to_gate);
    public:
        /** 
          *class constructor
          *@param a the composition id
          */
        Composition(const std::string &a) : 
        m_map(), 
        m_composition_id(a)
        {}
        
        std::string composition_id()
        {
            return m_composition_id;
        }

        /**
          * This is the only entry point for the c++ legacy version
          * It does not support reconfiguration
          * @param  the xml subtree containing the whole composition descriptions (including blocks and connections)
          */
        void install(const xml_node& n);


 // these functions are used by the BlockMon demon through the python binding, or are called by the legacy install method 
        /** 
          * Exposed through the python bindings
          * Creates a connection between two gates
          * @param from_block source block name
          * @param from_gate source gate name
          * @param to_block destination block name
          * @param to_gate destination gate name
          */

        void create_link(const std::string& from_block,const std::string& from_gate,const std::string& to_block,const std::string& to_gate);
        /** 
          * Exposed through the python bindings
          * Deletes a connection between two gates
          * @param from_block source block name
          * @param from_gate source gate name
          * @param to_block destination block name
          * @param to_gate destination gate name
          */
        void delete_link(const std::string& from_block, const std::string& from_gate,const std::string& to_block,const std::string& to_gate);

        
        
        /** 
          * Exposed through the python bindings
          * Creates a block (calls the private method create_block_from_parsed)
          * @param type the block type
          * @param name the name of the block instance
          * @param active boolean flag defining whether the block is active or passive
          * @param params  a human-readable string containing xml block-specific configuration subtree 
          */
        void create_block(const std::string& name,const std::string& type, bool active, const  std::string& config)
        {
            xml_document config_info;
            if(!config_info.load(config.c_str()))
                throw std::runtime_error("Composition::cannot parse config info");
            xml_node params = config_info.child("params");

            if (!params)
                throw std::runtime_error("Composition: cannot find params node");

            create_block_from_parsed( type, name, active, params);
        }

        
        /** 
          * Exposed through the python bindings
          * Delete a block in the composition
          * Notice that this function does not disconnect gates, which has to be done explicitly
          * otherwise there may be invalid references
          * @param name the block name
          */
        void delete_block(const std::string& name)
        {
            auto it = m_map.find(name);
            if(it == m_map.end())
                throw std::runtime_error("Composition:: trying to delete non-existing block");
            m_map.erase(it);
        }


        /** 
          * @return a reference to  a block in the composition
          * @param name the block name
          */
        std::shared_ptr<Block> get_block(const std::string& name)
        {
            auto it = m_map.find(name);
            if(it == m_map.end())
                throw std::runtime_error("Composition:: requested non-existing block");
            return it->second;
        }


    };

}//end namespace


#endif 
