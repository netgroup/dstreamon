#include <Composition.hpp>
#include <CompositionManager.hpp>

using namespace bm;


OutGate& Composition::find_output_gate( const std::string& from_block,const std::string& from_gate)
{
    auto map_it = m_map.find(from_block);
    if(map_it == m_map.end())
        throw std::runtime_error(std::string(from_block).append(" non existent block in link"));
    std::shared_ptr<Block> src_block = map_it->second;
    return src_block->get_output_gate(from_gate); //throws if gate is missing
}


InGate& Composition::find_input_gate( const std::string& to_block,const std::string& to_gate)
{
    auto map_it = m_map.find(to_block);
    if(map_it == m_map.end())
        throw std::runtime_error(std::string(to_block).append(" non existent block in link"));
    std::shared_ptr<Block> dst_block = map_it->second;
    return dst_block->get_input_gate(to_gate); //throws if gate is missing
}




void Composition::create_link(const std::string& from_block,const std::string& from_gate ,const std::string& to_block,const std::string& to_gate)
{
    OutGate& from = find_output_gate(from_block,from_gate);
    InGate& to = find_input_gate(to_block,to_gate);
    to.connect(from);
    from.connect(to);

}
void Composition::delete_link(const std::string& from_block,const std::string& from_gate ,const std::string& to_block,const std::string& to_gate)
{
    OutGate& from = find_output_gate(from_block,from_gate);
    InGate& to = find_input_gate(to_block,to_gate);
    to.disconnect(&from);
    from.disconnect(&to);
}

void Composition::create_block_from_parsed(const std::string& type, 
                                  const std::string& name, 
                                  bool active, 
                                  const xml_node & params)
{
    std::shared_ptr<Block> sp = BlockFactory::instantiate(type, name, active);
    if (!sp) {
        throw std::runtime_error(std::string(type).append(": block type not supported"));
    }
    sp->configure(params);
    m_map[name] = std::move(sp);
}



void Composition::install(const xml_node& config)
{
    for (xml_node blocknode=config.child("block"); blocknode; blocknode=blocknode.next_sibling("block"))
    {
        std::string name = blocknode.attribute("id").value();
        std::string type = blocknode.attribute("type").value();

        if((name.length() == 0) || (type.length() == 0)){
            throw std::runtime_error("block name or type missing");
        }
        std::string threadpool=blocknode.attribute("threadpool").value();
        // FIXME consider calling me "active"
        std::string sched_type=blocknode.attribute("sched_type").value();

        bool active = false;
        if (sched_type.length() > 0) {
            active=sched_type.compare("active") == 0;
            if(!active) throw std::runtime_error("sched_type val invalid");
        }

        xml_node args=blocknode.child("params");
        create_block_from_parsed(type, name, active, args);
        if (active) {
            if(threadpool.length()==0) throw std::runtime_error("could not find thread pool id");
            PoolManager::instance().add_to_pool(threadpool,m_map[name]);
        }
    }

    for (xml_node link=config.child("connection"); link; link=link.next_sibling("connection"))
    {
        std::string from_block=link.attribute("src_block").value();
        if (from_block.length() == 0) throw std::runtime_error("missing begin block in connection");
        
        std::string from_gate=link.attribute("src_gate").value();
        if (from_gate.length() == 0) throw std::runtime_error("missing begin gate in connection");
        
        std::string to_block=link.attribute("dst_block").value();
        if (to_block.length() == 0) throw std::runtime_error("missing dst block in connection");
        
        std::string to_gate=link.attribute("dst_gate").value();
        if (to_gate.length() == 0) throw std::runtime_error("missing begin gate in connection");
        
        create_link(from_block, from_gate, to_block, to_gate);
    }

}

