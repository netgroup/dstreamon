

// #pragma once

#ifndef __TIMEOUT_HPP__
#define __TIMEOUT_HPP__

#include <cassert>
#include <time.h>
#include <boost/heap/fibonacci_heap.hpp>

#include <streamon/Streamon.hpp>

#define QUEUE_TYPE boost::heap::fibonacci_heap
#define QUEUE_COMP boost::heap::compare

struct timeout_event
{
    uint64_t ExpireTime;
    uint32_t Value;
    HashedBuffer<128>* PrimaryFlowkey;
    void* Context;// std::shared_ptr<const bm::Msg> Context;
    void* Node;

    uint16_t Type;

    struct comparator
    {
        bool operator()(const timeout_event& a, const timeout_event& b) const
        {
            // std::cout << "comparator called\n";

            return (a.ExpireTime > b.ExpireTime);
        }
    };
};


typedef QUEUE_TYPE<timeout_event, QUEUE_COMP<timeout_event::comparator> >::handle_type timeout_handle;


class TimeoutManager
{
  
  QUEUE_TYPE<timeout_event, QUEUE_COMP<timeout_event::comparator> > queue;

  std::vector< DLeft<timeout_handle> * > timeout_tables;

public:

  TimeoutManager(size_t num_of_tables=1) : timeout_tables(num_of_tables)
  {       
    for (size_t i=0; i < num_of_tables; i++)
    {
      timeout_tables[i] = new DLeft<timeout_handle>(8, 20);
    }
  }


  void Set(IReadBuffer& key,
           uint32_t value, 
           void* context, // const std::shared_ptr<const bm::Msg>& context,
           uint16_t type,
           HashedBuffer<128>* primary_key = NULL);


  void Remove(IReadBuffer& key, uint16_t type);


  bool Pop( uint64_t time, timeout_event* out );


  void Free( void* node, uint16_t type );

};


#endif // __TIMEOUT_HPP__
