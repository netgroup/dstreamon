
#pragma once

#include<pthread.h>


class Thread_lock
{
 
 public:
  
    virtual void lock(bool w)=0;//if w=true lock in write mode 
    virtual void unlock()=0;
    static Thread_lock* init_lock();
 
    virtual ~Thread_lock()
    {
    }
};
