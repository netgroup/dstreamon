/**
 * <blockinfo type="Info_Block" is_active="False" thread_exclusive="False">
 *   <humandesc>
 *   Receives a Packet message and prints its associated information (as returned by the methods in the Packet class)
 *   </humandesc>
 *
 *   <shortdesc>Prints meta-information regarding a packet</shortdesc>
 *
 *   <gates>
 *     <gate type="input" name="in_pkt" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      }
 *   </paramsschema>
 *
 *   <paramsexample>
 *     <params>
 *     </params>
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */

#include <Block.hpp>
#include <BlockFactory.hpp>
#include <Packet.hpp>
#include <ClassId.hpp>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include "PacketPlus.hpp"
#include "CounterIP_S.hpp"
#include <streamon/pptags.h>

#include <iomanip>

namespace bm
{
  extern int g_pkt_count;

  class TestBlock : public Block
  {
    
  public:
    /*
     * costruttore
     */
    TestBlock(const std::string &name, bool) : Block(name, false),
						  m_ingate_id(register_input_gate("in_gate")),
						  m_outgate_id(register_output_gate("out_gate"))
    {
    }

    TestBlock(const TestBlock &) = delete;					
    TestBlock& operator=(const TestBlock &) = delete;			
    TestBlock(TestBlock &&)=delete;						
    TestBlock& operator=(TestBlock &&) = delete;				
    
    /*
     * distruttore
     */
    virtual ~TestBlock()
    {}
    
    /*
     * questo blocco nn ha parametri di configurazione
     */
    
    virtual void _configure(const xml_node&  n ) {
                
    }
			
    /*
     * il metodo principale
     * riceve un messaggio Packet e stampa delle informazioni sul pacchetto
     * se il messaggio appartiene ad un altro tipo viene catturata una eccezione.
     */
    virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /*id_ingate*/) {

        /* get packet */

        auto CurrentPacket = static_cast<const MonstreamMessage *>(m.get());

        std::cout << "PACKET ARRIVED!\n";

        // auto Tags = CurrentPacket->getTags();

        // auto pTable = Tags->Tables[0];

        // if (pTable == NULL) return;

        // auto DomainFlowkey = CurrentPacket->getFlowkey(DOMAIN_NAME_F);

        /* THE ONLY ONE METHOD */

        // pTable->add( *DomainFlowkey, "" );

        // pTable->remove( *DomainFlowkey, DomainFlowkey->GetHash() ); 

        CurrentPacket->~MonstreamMessage();

        // send_out_through(std::move(m), m_outgate_id);
    }

  private:
    
    int m_ingate_id;
    int m_outgate_id;
            
    // state_tuple* StatusList;
  };

  REGISTER_BLOCK(TestBlock,"TestBlock");
}
