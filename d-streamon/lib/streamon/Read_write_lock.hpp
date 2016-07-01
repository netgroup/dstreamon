#pragma once

#include"Thread_lock.hpp"

class Read_write_lock:public Thread_lock
{
 
 private: pthread_rwlock_t mutex;

 public:
    Read_write_lock();
    virtual void lock(bool w);
    virtual void unlock();
    

};
