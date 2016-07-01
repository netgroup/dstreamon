
#pragma once

#include"Thread_lock.hpp"

class Mutex:public Thread_lock
{
 
 private: pthread_mutex_t mutex;

 public:
    Mutex();
    virtual void lock(bool w);
    virtual void unlock();
    

};
