
/**
 * <blockinfo type="IPFIXExporter" scheduling_type="False" thread_exclusive="True" thread_safe="False">
 *   <humandesc>
 *  This blocks received a message and exports through an IPFIX channel.
 *  This may be a TCP connection, a UDP message, or a file
 *  The details of the IPFIX templates associated to an internal blockmon message are retrieved from the
 *  factory in MsgRegistry and kept in a local cache.
 *  Notice that this block can handled whatever message class includes a registration to the message registry.
 *  If the registry entry for the message type is not found, an exception is thrown.
 *   </humandesc>
 *
 *   <shortdesc>Exports BlockMon internal messages by using IPFIX</shortdesc>
 *
 *   <gates>
 *     <gate type="input" name="in_msg" msg_type="Msg" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *       element domain {xsd:integer},
 *       element export {
 *           attribute transport {"udp"|"tcp"|"sctp"},
 *           attribute host  {text},
 *           attribute port {xsd:integer}?
 *           } |
 *       element file {
 *           attribute name {text}
 *       }
 *      }
 *   </paramsschema>
 *
 *   <paramsexample>
 *     <params>
 *       <domain id="31"/>
 *       <export host="131.114.54.11" port="113" transport="udp"/>
 *       or
 *       <file name="dump"/>
 *     </params>
 *   </paramsexample>
 *      
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include <Block.hpp>
#include <FileWriter.h>
#include <Exporter.h>
#include <UDPExporter.h>
#include <MsgRegistry.hpp>
#include <BlockFactory.hpp>


using namespace pugi;

namespace bm {

class IPFIXExporter : public Block {

    struct local_cache_entry
    {
        IPFIX::StructTemplate tmpl;
        uint16_t tmpl_id;
    };


    int m_in_gate_id;
    std::unique_ptr<IPFIX::Exporter> m_exporter;
    std::unordered_map<int,std::shared_ptr<local_cache_entry> > m_local_cache;
    std::vector<uint8_t> m_local_buffer;

    void register_msg_templates()
    {
        MsgRegistry& registry = MsgRegistry::instance();
        std::vector<int> msg_ids= registry.available_msg_ids();
        for (auto it = msg_ids.begin(); it != msg_ids.end() ; ++it) {
            const Msg::Descriptor& desc = registry.get_message_descriptor(*it);
            local_cache_entry* new_entry= new local_cache_entry;
            desc.fill_pod_template(new_entry->tmpl);
            new_entry->tmpl_id = desc.template_id();

            //update the local cache
            m_local_cache[*it] = std::shared_ptr<local_cache_entry> (new_entry);

            //update the exporter wire templates
            m_exporter->getTemplate(new_entry->tmpl_id)->mimic(new_entry->tmpl);
        }
        m_exporter->exportTemplatesForDomain();
    }



            


public:
 
   IPFIXExporter(const std::string& name, bool active):
   Block(name, active),
   m_in_gate_id( register_input_gate("in_msg") ),
   m_exporter(),
   m_local_cache(),
   m_local_buffer(1024)
   {}

//FIXME destructor , should it be something specific?   
   ~IPFIXExporter()
   {}

/**
 * Configure the block given an XML element containing configuration.
 * Called before the block will begin receiving messages.
 *
 * MUST be overridden in a derived class. 
 * Configuration errors should be signaled by throwing.
 *
 * @param xmlnode the <params> XML element containing block parameters
 */
   virtual void _configure(const xml_node& xmlnode) {
       uint32_t        odid = 1;
       xml_node        domspec, outspec;

       // FIXME
       // here we initialize the information model
       // we need to make sure this only gets done once
       IPFIX::InfoModel::instance().defaultIPFIX();

       // Don't support reconfiguration yet
       if (m_exporter) {
           throw new std::logic_error("IPFIX block not reconfigurable");
       }

       // Handle optional things first
       // Observation domain ID (defaults to 1).
       if ((domspec = xmlnode.child("domain"))) {
           std::string domid = domspec.attribute("id").value();

           if (domid.empty()) {
               throw std::runtime_error("Domain missing ID");
           }
           odid = boost::lexical_cast<uint32_t>(domid);
       }

       if ((outspec = xmlnode.child("export"))) {
           // Parse export (transport, host, port)
           std::string transport = outspec.attribute("transport").value();
           std::string host = outspec.attribute("host").value();
           std::string port = outspec.attribute("port").value();
           if (transport.empty() || host.empty()) {
               throw std::runtime_error("Export specification incomplete");
           }
           if (port.empty()) {
               port = std::string("4739");
           }

           // Create exporter based on transport
           if (transport == std::string("udp")) {
               std::cerr << "will export to udp " << host << ":" << port << std::endl;
               m_exporter = std::unique_ptr<IPFIX::Exporter> (new IPFIX::UDPExporter(host, port, odid));
           } else if (transport == std::string("tcp")) {
               throw std::runtime_error("TCP not yet supported");
               //m_exporter = new IPFIX::TCPExporter(host, port, odid);
           } else if (transport == std::string("sctp")) {
               throw std::runtime_error("SCTP not yet supported");
               //m_exporter = new IPFIX::SCTPExporter(host, port, odid);
           } else {
               throw std::runtime_error("Unsupported transport");
           }        
       } else if ((outspec = xmlnode.child("file"))) {
           // Parse file writer
           std::string filename = outspec.attribute("name").value();
           if (filename.empty()) {
               throw std::runtime_error("File specification missing name");
           }
           std::cerr << "will export to file " << filename << std::endl;
           m_exporter = std::unique_ptr<IPFIX::Exporter> (new IPFIX::FileWriter(filename, odid));
       } else {
           throw std::runtime_error("No export or file specification");
       }
       register_msg_templates();

   }

/**
 * Receive a BlockMon message
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
   virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int gate_id) 
   {
       auto it = m_local_cache.find(m->type());
       if(it == m_local_cache.end())
           throw std::runtime_error("IPFIX Exporter: message type not supported");
       //retrieve template information
       uint16_t msg_tmpl_id = it->second->tmpl_id;
       IPFIX::StructTemplate* msg_template = &it->second->tmpl;
       //create a pod version 
       size_t msg_podsize = m->pod_size();
       if(m_local_buffer.size() < msg_podsize)
           m_local_buffer.resize(msg_podsize);
       mutable_buffer<uint8_t> tmp_buffer (&m_local_buffer[0], m_local_buffer.size());
       m->pod_copy(tmp_buffer);
       //actually export
       m_exporter->setTemplate(msg_tmpl_id);
       m_exporter->exportRecord(*msg_template, &m_local_buffer[0]);
  }
 


  
  
};


REGISTER_BLOCK(IPFIXExporter,"IPFIXExporter")
}
