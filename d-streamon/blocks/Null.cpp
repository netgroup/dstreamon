
/**
 * <blockinfo type="Null" scheduling_type="False" thread_exclusive="False">
 *   <humandesc>
 *   Debug blocks which discards every message it receives.
 *   It takes no configuration parameters.
 *   </humandesc>
 *
 *   <shortdesc>Debug block discarding messages</shortdesc>
 *
 *   <gates>
 *     <gate type="input" name="in_msg" msg_type="Msg" m_start="0" m_end="0" />
 *   </gates>
 *
 *   <paramsschema>
 *    element params {
 *    }
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

namespace bm
{

    class Null: public Block
    {
        
        
    public:

        /*
         * constructor
         */
        Null(const std::string &name, bool)
        : Block(name, false),
          m_ingate_id(register_input_gate("in_msg"))
        {
        }

        Null(const Null &)=delete;
        Null& operator=(const Null &) = delete;

        /*
         * destructor
         */
        virtual ~Null()
        {}


        virtual void _configure(const xml_node&  /*n*/ )
        {
        
        }

            
    private:
        
        int m_ingate_id;
 
    };


    REGISTER_BLOCK(Null,"Null");

}

