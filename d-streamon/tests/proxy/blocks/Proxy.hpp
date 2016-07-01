/* 
 * File:   Proxy.hpp
 * Author: fabrizio
 *
 * Created on 2 maggio 2011, 23.18
 */

#ifndef PROXY_HPP
#define	PROXY_HPP
#define DEFAULT_PORT 1001
#define BUFFER_LENGTH 1500

#include <Block.hpp>
#include <pugixml.hpp>
#include <BlockFactory.hpp>
#include <Buffer.hpp>
#include <MarshFactory.hpp>
#include <string>
#include <vector>
#include <sys/socket.h>



namespace bm
{
    class Proxy: public Block
    {
        
        bool m_live,th_safe;
        int m_gate_out,m_gate_in;
        std::string server_addr;
        short server_port;
        int server_sock;
        std::vector<std::string> out_addr;
        int out_sock;
        std::vector<short> out_port;
        char *buffer_in;
        char *buffer_out;
        


        Proxy(const Proxy &) = delete;
        Proxy& operator=(const Proxy &) = delete;

    public:

        Proxy(const std::string &name, bool active)
        :Block(name, active),
         m_live(active),
         m_gate_out(0),
         m_gate_in(0),
         server_addr(""),
         server_port(0),
         server_sock(0),
         out_addr(),
         out_sock(0),
         out_port(),
         buffer_in(NULL),
         buffer_out(NULL)
        {}

        virtual ~Proxy()
        {
            delete[] buffer_in;
            delete[] buffer_out;
        }


        virtual void _configure(const xml_node& n);

        virtual void _receive_msg(std::shared_ptr<const Msg>&&  m , int gateid);

        virtual void _do_async_processing();
        
    };

    REGISTER_BLOCK(Proxy,"proxy");
}


#endif	/* PROXY_HPP */

