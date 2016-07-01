
#include <Block.hpp>
#include <BlockFactory.hpp>
#include <Packet.hpp>
#include <ClassId.hpp>
#include <arpa/inet.h>
#include <sstream>
#include <unordered_map>
#include <PacketPlus.hpp>
#include "Metric_Block.hpp"

#include <streamon/Streamon.hpp>
#include <streamon/Thread_lock.hpp>
#include <lib_botstream/Timeout.hpp>

#include <dlfcn.h>

namespace bm
{
    DLeft<state_data> states_map(8, 20);

    TimeoutManager* tmanager;

	Thread_lock* stateLock = NULL;

    class EventFactory: public Block
    {
    protected:
        const event* find_event(pptags* Tags) const
        {
            // timeout_event* tev = Tags->TEvent;

            if (Tags->TEventId != -1)
            {
                /*
                if (tev->PrimaryFlowkey)
                    Tags->StateFlowkey = tev->PrimaryFlowkey;
                */

                return &EventTable[ Tags->TEventId ];
            }


            for (event* ev = EventTable; ev->Match != NULL; ev++)
            {
                if ( ev->Match(Tags) ) return ev;
            }

            return NULL;
        }

        bool build_metric_ops(const MonstreamMessage* pktp) 
        {
            auto Tags = pktp->getTags();

            metric_op* MetricOps = Tags->State->MetricSet;

            if (MetricOps == NULL) return false;

            /* for each Op in StateInfo.MetricOps */
            while ( MetricOps->MetricID != -1 )
            {
                calculate_flowkey( pktp, MetricOps->VDFlowkeyGet );
                calculate_flowkey( pktp, MetricOps->VDFlowkeySet );

                calculate_flowkey( pktp, MetricOps->VMFlowkeySet );
                calculate_flowkey( pktp, MetricOps->VMFlowkeyGet );

                Tags->MetricOps[ MetricOps->MetricID ] = MetricOps;

                MetricOps++;
            }

            return true;
        }

    public:
        /*
         * costruttore
         */
        EventFactory(const std::string &name, bool) : Block(name, false),
                                                      m_ingate_id(register_input_gate("in_Factory")),
                                                      m_outgate_id(register_output_gate("out_Factory")),
                                                      BuiltinTokens(0),
                                                      LastTokenId(0)
        {
            pkt_count = 0;

            /*## LOAD SHARED LIBRARY ##*/

            LibHandle = dlopen("./FeatureLib.so", RTLD_LAZY);

            if (LibHandle == NULL)
            {
                throw std::runtime_error(dlerror());
            }

            /* Loading event table */

            EventTable = (event*) dlsym(LibHandle, "Events");

            if (EventTable == NULL)
            {
                throw std::runtime_error(dlerror());
            }

			if (stateLock == NULL)
			{
				stateLock = Thread_lock::init_lock();
			}
        }

        EventFactory(const EventFactory &) = delete;				
        EventFactory& operator=(const EventFactory &) = delete;			
        EventFactory(EventFactory &&)=delete;					
        EventFactory& operator=(EventFactory &&) = delete;				

        /*
         * distruttore
         */
        virtual ~EventFactory()
        {}

        /*
         * questo blocco ha parametri di configurazione
         */
        virtual void _configure(const xml_node&  n )
        {

            /*## PARSE FIELDS ## */

            for (xml_node f = n.child("field"); f; f = f.next_sibling("field") )
            {
                std::string name = f.attribute("name").value();

                int id = f.attribute("id").as_int();

                if (id == -1) continue;

                TokenMap[name] = id;

                if (id > LastTokenId)
                    LastTokenId = id;
            }

            assert(LastTokenId > 0);

            BuiltinTokens = LastTokenId + 1;


                /* DEPRECATED!!! ## PARSE ADDITIONAL TABLES ##


                   for (xml_node t = n.child("table"); t; t = t.next_sibling("table") )
                   {
                   int nhash = 8;
                   int shash = 20;

                   auto id_attr = t.attribute("id");
                   auto nh_attr = t.attribute("nhash");
                   auto sh_attr = t.attribute("shash");

                   std::string id_token = id_attr.value();
                   std::string nh_token = nh_attr.value();
                   std::string sh_token = sh_attr.value();

                   if ( id_token == "" )
                   throw std::runtime_error("Botstream config: error parsing table, ID not found");

                   int id = id_attr.as_int();

                   if ( nh_token != "" )
                   nhash = nh_attr.as_int();

                   if ( sh_token != "" )
                   shash = sh_attr.as_int();

                // Create table
                tables[id] = new DLeft<std::string>(nhash, shash);

                // check if there are entries to add

                for (xml_node e = t.child("entry"); e; e = e.next_sibling("entry") )
                {
                std::string key = e.attribute("key").value();

                std::string value = e.attribute("value").value();

                // std::cout << "WARNING: table pre-inserts are DISABLED!!!\n";
                // int value = e.attribute("value").as_int();

                std::cout << "K => " << key << "\n";
                std::cout << "V => " << value << "\n";

                auto hkey = HashedReadBuffer( key );

                tables[id]->add( hkey, value);
                }
                }
                 */

                /* assert( TokenMap.size() == NUM_OF_PP_FIELDS ) */

                /*## PARSE ADDITIONAL KEYS ##*/

            for (xml_node k = n.child("additional_key"); k; k = k.next_sibling("additional_key") )
            {
                std::string key_name = k.attribute("name").value();

                if (key_name != "")
                {
                    int key_index = TokenMap[key_name];

                    std::cout << "Additional key " << key_name << " at index " << key_index << "\n";

                    tokenize(key_name, FlowkeyRules[LastTokenId-key_index]);

                    std::cout << "[FACTORY] Added flowkey rule at index => " << LastTokenId-key_index << "\n"; 

                    BuiltinTokens--;
                }
            }

    
            /*## PARSE TIMEOUT CLASSES NUMBER ## */

            auto tclasses = n.child("timeouts").attribute("classes").as_int();

            if (tclasses > 0)
            {
                tmanager = new TimeoutManager(tclasses);

                set_periodic_timer(std::chrono::milliseconds(100), "timeout_watcher", 0);
            }            
        }

		virtual void _handle_timer(std::shared_ptr<Timer>&& t)
        {
            timespec ts;

            clock_gettime(CLOCK_REALTIME, &ts);

			uint64_t timestamp_us = ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;

			// std::cout << "[Timer Thread] Timestamp => " << timestamp_us << "\n";

            timeout_event tmsg;

            while ( tmanager->Pop( timestamp_us, &tmsg ) )
            {			    
                auto Tags = (struct pptags*) tmsg.Context;
                
                std::shared_ptr<const Msg> m = Tags->Message;

                auto smsg = static_cast<const MonstreamMessage*>( m.get() );

                assert(smsg);

                if (tmsg.Value == 0)
                {
                    // deallocate
                    
                    smsg->~MonstreamMessage();
                    
                    //std::cout << "[ TIMEOUT REMOVED ]\n";
                }
                else
                {
                    // update fields

                    memset(Tags->MetricOps, 0, 16*sizeof(metric_op*));

                    Tags->TEventId = tmsg.Type;

					//std::cout << "[Timer Thread] Timestamp expired => " << tmsg.ExpireTime << "\n";

					/*				
					timespec nt;
					nt.tv_sec = tmsg.ExpireTime / 1000000ULL;
					nt.tv_nsec = (tmsg.ExpireTime - nt.tv_sec * 1000000ULL) * 1000ULL;

                    smsg->timestamp_set(nt);
					*/

					//std::cout << "ts.tv_sec: " << ts.tv_sec << std::endl;

                    _receive_msg( std::move(m), -1 );
                }
            }
        }       
			
        /*
         * il metodo principale
         * riceve un messaggio Packet e stampa delle informazioni sul pacchetto
         * se il messaggio appartiene ad un altro tipo viene catturata una eccezione.
         */
        virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /*index*/)
        {
            /*
               if ( m->type() == MSG_ID(PacketPlus) ) { }

             */

            pkt_count++;

            auto pktp = static_cast<const MonstreamMessage*>(m.get());

            pptags* Tags = pktp->getTags();

            Tags->TManager = tmanager;

            Tags->ResetMetrics = Metric_Block::reset_ops;

            // Tags->Tables = tables;

            auto state_key = Tags->StateFlowkey;

            const event* EventData = find_event(Tags);

            if ( EventData == NULL )
            {
                /* drop packet */
                pktp->~MonstreamMessage();

                return;
            }


			// SET TIMESTAMP
			// Note: this is needed on live capture to avoid
			//       deltat assert fails into tewma
			
			timespec ts;

            clock_gettime(CLOCK_REALTIME, &ts);

			pktp->timestamp_set(ts);


            // UPDATE ATTACK TYPE
            Tags->Attack.Id = EventData->AttackType;

            // PRIMARY KEY SELECTION

            // state key retrieval
            if (state_key == NULL)
            {
                // compute primary key (if needed)
                calculate_flowkey(pktp, EventData->KeyPair.RegId);
                
                state_key = Tags->StateFlowkey = pktp->getFlowkey( EventData->KeyPair.RegId );

                    /* PRIMARY KEY FROM TABLE ACTUALLY DISABLED!!!

                    if (Key->TableId >= 0)
                    {
                        // std::cout << "Source of primary key is not packet map\n";

                        // QUERY AND COPY FROM TABLE
			      
                        // 1) GET SOURCE FLOWKEY
			      
                        auto SourceKey = pktp->getFlowkey(KeyID);
				
                        // 2) QUERY THE TABLE WITH TABLEID WITH (1)

                        auto table = tables[Key->TableId];

                        if (table == NULL)
                            throw std::runtime_error("Table index not valid");
			      
                        auto table_entry = table->get(*SourceKey);

                        if (table_entry == NULL)
                        {
                            std::cout << "dropping due to TABLE ENTRY NOT FOUND\n";
                            pktp->~MonstreamMessage();
                            return; // drop packet
                        }

                        // 3) COPY THE CONTENT OF THE TABLE INTO A MAP REGISTER!

                        // (a) found free entry register
                        // (b) use it

                        int register_index = LastTokenId + 1;

                        // std::cout << "First free register index => " << register_index << "\n";

                        Tags->Map[register_index] << *table_entry;

                        KeyID = register_index;
                    }
                    */
            }

			stateLock->lock(false);

            auto state_data = Tags->StateInfo = states_map.get( *state_key );

            auto StateId = ( state_data ? state_data->Id : 0 );

            Tags->State = &EventData->StateTable[ StateId ];

            if ( StateId > 0 and
                    state_data->Expire > 0 and
                    pktp->timestamp_us() > state_data->Expire )
            {
                // take another state

                std::cerr << "state timeout from: " << StateId;

                StateId = state_data->Id = Tags->State->TimeoutState;

                std::cerr << ", to: " << StateId << "\n";

                Tags->State = &EventData->StateTable[ StateId ];

                if (StateId == 0)
                {
                    // remove entry from dleft

					// stateLock->lock(true);

                    states_map.remove(*state_key);

					// stateLock->unlock();

                    Tags->StateInfo = NULL;
                }
                else {
                    if (Tags->State->Timeout > 0) {
                        state_data->Expire = pktp->timestamp_us() + Tags->State->Timeout;
                    }
                    else {
                        state_data->Expire = 0;
                    }
                }
            }

			stateLock->unlock();

            if ( !build_metric_ops(pktp) )
            {
                pktp->~MonstreamMessage();
                return; /* drop packet */
            }

            send_out_through(std::move(m), m_outgate_id);
        }

    private:

        /*
         *parser per riempire di flowkey il vettore variation/monitor detector
         */
        void tokenize(std::string& w, std::vector<int>& v){

            int point = 0;
            int size  = 0;
            for(unsigned int i = 0; i < w.length(); i++){

                size++;
                if(w.substr(i,1) == "-"){

                    //std::cout<<w.substr(point,size-1)<<"\n";
                    v.push_back(TokenMap[w.substr(point,size-1)]);
                    point = i+1;
                    size  = 0;
                }
            }
            v.push_back(TokenMap[w.substr(point,size)]);
            //std::cout<<w.substr(point,size)<<"\n";
        }

        inline void calculate_flowkey( const MonstreamMessage* pktp, int flowkey_id )
        {

            if ( flowkey_id < BuiltinTokens ) return;

            auto CurrentFlowkey = pktp->getFlowkey(flowkey_id);

            if (CurrentFlowkey->GetLength() == 0)
            {
                /* note that this is true only for combo-flowkeys 
                 * as normal flowkeys doesn't have composition rules
                 */

                std::vector<int>& Rule = FlowkeyRules[LastTokenId-flowkey_id]; // NUM_OF_PP_FIELDS];

                for (auto field : Rule)
                {
                    Buffer<REGISTER_SIZE>* b = pktp->getFlowkey(field);

                    try
                    {
                        // std::cout << "before insert\n";
                        *CurrentFlowkey << *b;
                    }
                    catch (std::out_of_range& ex)
                    {
                        std::cout << "error inserting\n";
                    }
                }
            }
        }
			
        int m_ingate_id;
        int m_outgate_id;
        int pkt_count;

        void* LibHandle;

        event* EventTable;

        std::unordered_map<std::string, int> TokenMap;

        int BuiltinTokens;
        int LastTokenId;

        std::vector<int> FlowkeyRules[8];
    };

    REGISTER_BLOCK(EventFactory,"EventFactory");
}
