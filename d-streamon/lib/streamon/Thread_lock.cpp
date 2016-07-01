
#include"Mutex.hpp"
#include"Spinlock.hpp"
#include"Nolock.hpp"
#include"Read_write_lock.hpp"

Thread_lock* Thread_lock::init_lock()
{
    return new Nolock();
}
