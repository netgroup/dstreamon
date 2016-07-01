#include "zhelpers.hpp"

#include <fstream>

#define PID_FILE "zmq_client.pid"

int main ( int argc, char *argv[] ) {

    pid_t zmq_client_pid;

    zmq_client_pid = getpid();

    //fprintf(stderr, "%i\n", zmq_client_pid );

    std::ofstream pid_file(PID_FILE);

    pid_file << zmq_client_pid;

    pid_file.close();


    //  Prepare our context and subscriber
    zmq::context_t context(1);
    zmq::socket_t subscriber (context, ZMQ_SUB);
    //subscriber.connect("tcp://localhost:5560");

    if( argc == 1 ) {
        fprintf( stdout, "Per eseguire il programma bisogna inserire l'host e la porta a cui sottoscriversi\n");
        exit(0);
    }

    else if ( argc == 2 )
    {
        //printf( "Host inserito %s \n", argv[1]);
        subscriber.connect(argv[1]);
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