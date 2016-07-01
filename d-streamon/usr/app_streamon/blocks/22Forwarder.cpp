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
// #include "CounterIP_S.hpp"
#include <streamon/pptags.h>
#include <streamon/SocketCUnix.h>
#include <iomanip>
#include <streamon/Writer.h>
#include "FeedPacket.h"

namespace bm
{
	extern int g_pkt_count;
	int c = 0;
	class Forwarder : public Block
	{
		
		struct feed_conf {
			std::string attack_type;
			int fname;
			int fcontent;
			int export_flag;
		};
		
		std::vector <feed_conf> feeds;
		char buffer[1500];
		size_t xml_writer(std::string a_type, std::string f_name, std::string f_content, int alert)
		{			
                    // For this example, we'll start with an empty document and create nodes in it from code
                    xml_document doc;

                    // Append several children and set values/names at once
                    doc.append_child(node_comment).set_value("Export To CEDE");
                    doc.append_child().set_name("feed");

                    // Let's add a few modules
                    xml_node message = doc.child("feed");

                    message.append_attribute("attack_type").set_value(a_type.c_str());
                    message.append_attribute("name").set_value(f_name.c_str()); 
                    message.append_attribute("content").set_value(f_content.c_str());
                    message.append_attribute("alert").set_value(alert);

                    auto buf_size = sizeof(buffer);

                    xml_memory_writer writer(buffer, buf_size - 1);
                    doc.child("feed").print(writer);

                    return writer.written_size();
		}
		
		
	public:
		/*
		 * costruttore
		 */
		Forwarder(const std::string &name, bool) : Block(name, false),
		m_ingate_id(register_input_gate("in_gate")),
		m_outgate_id(register_output_gate("out_gate")),
		Client(true) // if true: nonblocking
		{
		}
		
		Forwarder(const Forwarder &) = delete;					
		Forwarder& operator=(const Forwarder &) = delete;			
		Forwarder(Forwarder &&)=delete;						
		Forwarder& operator=(Forwarder &&) = delete;	
		
		/*
		 * distruttore
		 */
		virtual ~Forwarder()
		{}
		
		/*
		 * questo blocco nn ha parametri di configurazione
		 */
		
		virtual void _configure(const xml_node&  n ) {

            auto sock_node = n.child("socket");

            if (sock_node)
            {
                std::string sock_path = sock_node.attribute("path").value();

                if (sock_path != "")
                    Client.start_connection(sock_path);
            }

			
			for (pugi::xml_node feed = n.child("feed"); feed; feed = feed.next_sibling("feed"))
			{
				feed_conf f;
				
				f.attack_type = feed.attribute("atype").value();
				f.fname  = feed.attribute("name").as_int();
				f.fcontent = feed.attribute("content").as_int();
				f.export_flag = feed.attribute("export").as_int();
				
				feeds.push_back(f);
				
			}
			
		}
		
		/*
		 * il metodo principale
		 * riceve un messaggio Packet e stampa delle informazioni sul pacchetto
		 * se il messaggio appartiene ad un altro tipo viene catturata una eccezione.
		 */
		virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /*id_ingate*/) {
			// std::cout << "--FORWARDER--\n";	
			/* get packet */
			
			auto CurrentPacket = static_cast<const MonstreamMessage *>(m.get());
			
			
			auto Tags = CurrentPacket->getTags();
			
			auto Attack = Tags->Attack;
			
			if (Attack.Id < 0) {
				// CurrentPacket->~MonstreamMessage();
				
				return;
			}

			feed_conf& f = feeds[Attack.Id];
			// printf("Attack.Suspect: %d\n", Attack.Suspect);
			if (f.export_flag == 1 && Attack.Suspect == 0) {
				// CurrentPacket->~MonstreamMessage();
				
				return;
			}
			
			
			auto& name = Tags->Map[f.fname];
			std::string feed_name = name.GetString();
			//std::cout << "Feed Name: " << feed_name << std::endl;			

	
			auto& content = Tags->Map[f.fcontent];

		    std::string feed_content = "";

		   if (Attack.Suspect == 0)
		    {
			feed_content = content.GetString();
		    }
			//std::cout << "Feed Content: " << feed_content << std::endl;			
			//Scrivere l'XML
			FeedPacket feedPacket;
			feedPacket.attackType = f.attack_type;
			feedPacket.name = feed_name;
			feedPacket.content = feed_content;
			feedPacket.isAlerted = Attack.Suspect;
			
			c++;
			
			// if ( Client.is_connected() )
			std::string packet = feedPacket.serialize();
                	while ( Client.send(packet.c_str(), packet.size()) == -1 && Attack.Suspect)
			{
				struct timespec ts = { 0, 1000000 };

				// std::cout << "FAILED TO ALERT CEDE.\n";

				nanosleep(&ts, NULL);
			}

			if (Attack.Suspect)
			{
				std::cout << "Send alert!\n";
			}
			// std::cout<<"Buffer: "<<buffer<<std::endl;	
			// send_out_through(std::move(m), m_outgate_id);
		}
		
	private:
		
		int m_ingate_id;
		int m_outgate_id;
		//Client Socket Unix
		SocketCUnix Client;
		
		// state_tuple* StatusList;
	};
	
	REGISTER_BLOCK(Forwarder,"Forwarder");
}
