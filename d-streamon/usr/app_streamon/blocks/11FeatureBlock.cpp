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
// #include <Packet.hpp>
#include <ClassId.hpp>
// #include <dlfcn.h>
#include <arpa/inet.h>
#include <sstream>
// #include <fstream>
#include <PacketPlus.hpp>
// #include "CounterIP_S.hpp"
#include <streamon/pptags.h>

#include <iomanip>

namespace bm
{
  extern int g_pkt_count;

  class FeatureBlock : public Block
  {
    
  public:
    /*
     * costruttore
     */
    FeatureBlock(const std::string &name, bool) : Block(name, false),
						  m_ingate_id(register_input_gate("in_FeatureBlock")),
						  m_outgate_id(register_output_gate("out_FeatureBlock"))
    {
    }

    FeatureBlock(const FeatureBlock &) = delete;					
    FeatureBlock& operator=(const FeatureBlock &) = delete;			
    FeatureBlock(FeatureBlock &&)=delete;						
    FeatureBlock& operator=(FeatureBlock &&) = delete;				
    
    /*
     * distruttore
     */
    virtual ~FeatureBlock()
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

      g_pkt_count++;

      double* MetricValues = NULL;
      int Len = 0;

      /* get packet plus */

      auto CurrentPacket = static_cast<const MonstreamMessage *>(m.get());

      auto Tags = CurrentPacket->getTags();

      /*
      auto IPPacket = (PacketPlus*) CurrentPacket;
      auto StateID = Tags->StateInfo == NULL ? 0 : Tags->StateInfo->Id;
      */

      CurrentPacket->getMetricsArray( &(MetricValues), Len );

      // std::cout << StateID << " ";

      // IPPacket->OutputWrite( std::cout );



      // std::cout << Tags->StateInfo->Id << " " << std::setw(14) << string_to_ip(CurrentPacket-> MetricValues[0] << " " << std::setw(14) << MetricValues[1] << " " << *CurrentPacket;

      /*std::cout << "PACKET => " << 



      for (int i=0; i < 4; i++)
      {
	std::cout << MetricValues[i] << " ";
      }

      std::cout << "\n";
      */

      /* Take the FeatureList by current Status */

      feature* FeatureList = Tags->State->FeatureList; // StatusList[CurrentPacket->get_state()].FeatureList;

      if (FeatureList == NULL)
      {
        send_out_through(std::move(m), m_outgate_id);
        return;
      }
      
      /* Execute Feature Functions */

      for (feature* Feat = FeatureList; Feat->Id != -1; Feat++)
      {
        CurrentPacket->setFeature( Feat->Id, Feat->Feature(MetricValues) );
      }
	
      send_out_through(std::move(m), m_outgate_id);
    }

  private:
    
    int m_ingate_id;
    int m_outgate_id;
            
    // state_tuple* StatusList;
  };

  REGISTER_BLOCK(FeatureBlock,"FeatureBlock");
}
