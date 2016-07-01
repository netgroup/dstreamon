

#include "Timeout.hpp"

using namespace boost::heap;



void TimeoutManager::Set(IReadBuffer& key,
                         uint32_t value, 
                         void* context,
                         uint16_t type,
                         HashedBuffer<128>* primary_key)
{
  auto timeout_map = timeout_tables[type];

  timeout_handle* timeout_data = timeout_map->get(key);

  timespec ts;
 
  // if timeout entry not present (new timeout set)
  if (timeout_data == NULL) 
  {
    // try to reserve key
    timeout_data = timeout_map->reserve(key);

    assert(timeout_data);

    clock_gettime(CLOCK_REALTIME, &ts);

    timeout_event ev;

    ev.ExpireTime     = (ts.tv_sec + value) * 1000000ULL + ts.tv_nsec / 1000ULL;
    ev.Value          = value;
    ev.PrimaryFlowkey = primary_key;
    ev.Context        = context;
    ev.Type           = type;
    ev.Node           = timeout_data;

    // save handle and push on queue the event
    *timeout_data = queue.push(ev);
  }
  else
  {
    // ENTRY EXISTS: UPDATE TIMEOUT 

    (**timeout_data).Value = value; // if value == 0: lazy delete at next pop

    if (value > 0)
    {
        // std::cout << "[TimeoutManager::Set] timeout update\n";
 
        clock_gettime(CLOCK_REALTIME, &ts);

        (**timeout_data).ExpireTime = (ts.tv_sec + value) * 1000000ULL + ts.tv_nsec / 1000ULL;

        queue.update(*timeout_data);
    }
  }
}


void TimeoutManager::Remove(IReadBuffer& key, uint16_t type)
{
  auto timeout_map = timeout_tables[type];
  auto timeout_data = timeout_map->get(key);

  if (timeout_data == NULL) return;

    (**timeout_data).Value = 0; // lazy delete
}


bool TimeoutManager::Pop( uint64_t time, timeout_event* out )
{
  if ( queue.empty() ) return false;

  timeout_handle watched = *(timeout_handle*) queue.top().Node;

  if ( (*watched).Value == 0)
  {    
    *out = *watched;
    
    Free( (*watched).Node, (*watched).Type );

    queue.pop();

    return true;
  }
  else
  if (time > (*watched).ExpireTime)
  {
    // reschedule timeout

    *out = *watched;

    (*watched).ExpireTime = time + (*watched).Value * 1000000ULL;

    queue.update(watched);

    return true;
  }

  return false;
}


void TimeoutManager::Free( void* node, uint16_t type )
{
    auto timeout_map = timeout_tables[ type ];

    timeout_map->remove( (timeout_handle*) node  );
}
