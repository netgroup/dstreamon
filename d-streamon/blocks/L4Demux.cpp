/**
 * <blockinfo type="L4Demux" scheduling_type="False" thread_exclusive="false">
 *   <humandesc>
 *   This blocks simply takes Packet messages into account and demultiplexes them across three possible output
 *   gates depending on their transport protocol.
 *   In particular, a TCP packets in forwarded through the out_tcp gate, a UDP through out_udp and any other packet
 *   through out_unknown.
 *   </humandesc>
 *
 *   <shortdesc>Demultiplexes packets based on their transport protocol</shortdesc>
 *
 *   <gates>
 *     <gate type="output" name="out_tcp" msg_type="Packet" m_start="0" m_end="0" />
 *     <gate type="output" name="out_udp" msg_type="Packet" m_start="0" m_end="0" />
 *     <gate type="output" name="out_unknown" msg_type="Packet" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *      }
 *    }
 *   </paramsschema>
 *
 *   <paramsexample>
 *   </paramsexample>
 *
 *   <variables>
 *   </variables>
 *
 * </blockinfo>
 */
#include <Block.hpp>
#include <Packet.hpp>
#include <BlockFactory.hpp>


namespace bm
{

    /**
     * Implements a block that demultiplexes packet messages based on their transport protocol
     * 0.5 seconds.
     */
    class L4Demux: public Block
    {
        int m_ingate_id;
        int m_out_tcp;
        int m_out_udp;
        int m_out_unknown;

    public:

        /**
         * @brief Constructor
         * @param name         The name of the packet counter block
         * @param active       Whether the block should be active or passive
         */
        L4Demux(const std::string &name, bool active)
        : Block(name, active), 
          m_ingate_id(register_input_gate("in_pkt")), 
          m_out_tcp(register_output_gate("out_tcp")), 
          m_out_udp(register_output_gate("out_udp")), 
          m_out_unknown(register_output_gate("out_unknown"))
        {}

        L4Demux(const L4Demux&)=delete;
        L4Demux(L4Demux&&)=delete;
        L4Demux& operator=(const L4Demux&)=delete;
        L4Demux& operator=(L4Demux&&)=delete;

        /**
         * @brief Configures the block: in this case does nothing as the block has no params 
         */
        virtual void _configure(const xml_node& /* n */)
        {
        }

        /**
         * @brief Destructor
         */
        virtual ~L4Demux()
        {}

        /**
         * If the message received is not of type Packet throw an exception,
         * otherwise demultiplexes it. 
         * @param m     The message
         * @param index The index of the gate the message came on
         */
        virtual void _receive_msg(std::shared_ptr<const Msg>&& m, int /* index */)
        {
            if(m->type()!=MSG_ID(Packet)) throw std::runtime_error("counter: unexpected msg type");
            const Packet* packet = static_cast<const Packet*>(m.get());
            
            if(packet->is_tcp()) 
            {
                send_out_through(std::move(m),m_out_tcp);
            } 
            else if(packet->is_udp())
            {
                send_out_through(std::move(m),m_out_udp);
            }
            else
            {
                send_out_through(std::move(m),m_out_unknown);
            }

        }

    };


    REGISTER_BLOCK(L4Demux,"L4Demux");

};//bm




