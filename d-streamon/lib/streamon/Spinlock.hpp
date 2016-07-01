
#pragma once

#include"Thread_lock.hpp"

class Spinlock:public Thread_lock
{
 
 private: pthread_spinlock_t spinlock;

 public:
    Spinlock();
    virtual void lock(bool w);
    virtual void unlock();
    

};




