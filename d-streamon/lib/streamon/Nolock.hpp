
#pragma once

#include"Thread_lock.hpp"
class Nolock:public Thread_lock
{
 
 
 public:
    Nolock(){}
    void lock(bool w){}
    void unlock(){}
    
};




