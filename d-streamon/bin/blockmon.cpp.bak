#include<CompositionManager.hpp>
#include<PoolManager.hpp>
#include<TimerThread.hpp>
#include <limits.h>
#include <signal.h>

using namespace bm;

bool signal_triggered = false;

void sig_handler(int signum)
{
	if (!signal_triggered)
	{
		std::cout<<"Stopping pool manager"<<std::endl;
		PoolManager::instance().stop();
		std::cout<<"Stopping timer thread"<<std::endl;

		TimerThread::instance().stop();

		signal_triggered = true;
	}
}


int main (int argc, char** argv)
{
    if(argc!=2)
        throw std::runtime_error("must specify configuration file");

    std::cout<<"add config\n";
    CompositionManager::instance().add_config_from_file(argv[1]);
    std::cout<<"done\n";

	struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler=sig_handler;

    sigaction(SIGINT,&sa,NULL);


	PoolManager::instance().start();

	std::thread tt(std::ref(TimerThread::instance()));

/*
	sleep(300); // ULONG_MAX);

*/

	tt.join();

	return (0);
}

