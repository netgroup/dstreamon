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
#include <streamon/Streamon.hpp>
#include <streamon/Thread_lock.hpp>
// #include "../../../scripts/actionlib.hpp"

namespace bm
{
  extern int g_pkt_count;

  extern Thread_lock* stateLock;

  extern DLeft<state_data> states_map;
  
  class FilteringBlock : public Block
  {
    
  public:
    /*
     * costruttore
     */
    FilteringBlock(const std::string &name, bool) : Block(name, false),
						    m_ingate_id(register_input_gate("in_FilteringBlock")),
						    m_outgate_id(register_output_gate("out_FilteringBlock"))
    {
    }

    FilteringBlock(const FilteringBlock &) = delete;					
    FilteringBlock& operator=(const FilteringBlock &) = delete;			
    FilteringBlock(FilteringBlock &&)=delete;						
    FilteringBlock& operator=(FilteringBlock &&) = delete;				
    
    /*
     * distruttore
     */
    virtual ~FilteringBlock()
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
    virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /*id_ingate*/)
    {
        state_tuple* State = NULL;

        condition_tuple* ConditionList = NULL;

        double* FeatureValues = NULL;
        int Len = 0;

        /* get packet plus */

        auto CurrentPacket = static_cast<const MonstreamMessage*>(m.get());

        CurrentPacket->getFeaturesArray( &(FeatureValues), Len );

        auto Tags = CurrentPacket->getTags();

        uint32_t Flags = 0x0;

        State = Tags->State;

        /* Take condition list */

        ConditionList = State->ConditionList; // CurrentState.ConditionList;
      
        /* Evaluate Condition Functions */

        bool Check = false;

        condition_tuple* CurrentTuple = ConditionList;
      
        for (; CurrentTuple->Condition != NULL && (Check == false); CurrentTuple++)
        {
            Check = CurrentTuple->Condition(FeatureValues, Tags);
        }

        CurrentTuple--;

        if (Check == true)
        {
            // Tags->Attack.Suspect = true;

            struct state_data* StateInfo = Tags->StateInfo;

            state_tuple* NextStateTuple = State;

            /* if change state */
            if (CurrentTuple->NextState != -1)
            {
				stateLock->lock(true);

                int StateID = (Tags->StateInfo ? Tags->StateInfo->Id : 0 );

                /* Using pointer arithmetics to find the next state tuple */
                NextStateTuple = (State - StateID + CurrentTuple->NextState);

                /* Preparing state entry (state, expire) */
                struct state_data StateData = { CurrentTuple->NextState, 0 };

                auto state_flowkey = Tags->StateFlowkey;
	  
                /* Update state */

                StateInfo = states_map.add( *state_flowkey, StateData );

				stateLock->unlock();
            }


            /* UPDATE EXPIRE TIME */
            if (StateInfo != NULL and NextStateTuple->Timeout > 0)
            {
				stateLock->lock(true);

                StateInfo->Expire = CurrentPacket->timestamp_us() + NextStateTuple->Timeout;

				stateLock->unlock();
            }

            action_function* Action = CurrentTuple->Action;

            if (Action != NULL)
            {
                /* Execute Actions */
	  
                for (; *Action != NULL; Action++)
                {
	                int ActionResult = (*Action)(CurrentPacket->getTags());

                    Flags |= ActionResult;
                }
            }
	
            // std::cout << g_pkt_count << "\t" << *CurrentPacket << std::endl;
        }

        /* Execute post condition action */
      
        action_function* Action = State->PostCondActionList;

        if (Action != NULL)
        {
            /* Execute Actions */
	  
            for (; *Action != NULL; Action++)
            {
	            int ActionResult = (*Action)(CurrentPacket->getTags());

                Flags |= ActionResult;
            }
        }    

    
        /* ### FLAGS HANDLING ### */

        if (Flags & TIMEOUT_SETTED)
        { 
            if (Tags->TEventId == -1)
            {
                // new timeout
                Tags->Message = m;
            }

            // avoid effects of timeout_removed
            // Flags &= ~TIMEOUT_REMOVED;
        }

        /* Here forward the packet if needed */

        if (Flags & EXPORT_PACKET)
        {
            send_out_through(std::move(m), m_outgate_id);
        }

        // waiting for send_out return, then cleanup if needed

        if ( Tags->Message == NULL ) //  || (Flags & TIMEOUT_REMOVED) )
        {
            // destroy message

            CurrentPacket->~MonstreamMessage();
        }
    }

  private:
    
    int m_ingate_id;
    int m_outgate_id;
  };

  REGISTER_BLOCK(FilteringBlock,"FilteringBlock");
}
