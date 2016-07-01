#include"Read_write_lock.hpp"


 Read_write_lock::Read_write_lock()
 {
    pthread_rwlock_init(&mutex,NULL);

 }
 void Read_write_lock::lock(bool w)
 {
   if(w==false)
    pthread_rwlock_rdlock(&mutex);
   else
    pthread_rwlock_wrlock(&mutex);
 }
 void Read_write_lock::unlock()
 {
   pthread_rwlock_unlock(&mutex);

 }




