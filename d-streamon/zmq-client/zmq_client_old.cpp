#include "zhelpers.hpp"

int main ( int argc, char **argv ) {
    //  Prepare our context and subscriber
    zmq::context_t context(1);
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5560");

    if ( argc == 1 )
    {
        fprintf( stdout, "ALL MSG\n");
        subscriber.setsockopt( ZMQ_SUBSCRIBE, "", 0);
    }
    else
    {
        fprintf( stdout, "FILTER MSG\n");

        subscriber.setsockopt(ZMQ_SUBSCRIBE, *(argv+1), strlen(*(argv+1)));
    }

    while (1) {
 
		//  Read envelope with address
		std::string channel = s_recv (subscriber);
		//  Read message contents
		std::string contents = s_recv (subscriber);
		
        if (channel != "raw")
        {
            std::cout << "[" << channel << "] " << contents << std::endl;
        }
        else
        {
            std::cout << "[" << channel << "]";
            for (int i = 0; i < contents.length(); ++i)
                std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(contents[i]));
            std::cout << std::endl;
        }
    }
    return 0;
}