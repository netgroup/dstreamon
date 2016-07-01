

#ifndef _ACTIONLIB_HPP
#define _ACTIONLIB_HPP

#include <iostream>
#include <stdio.h>

#include <streamon/pptags.h>
#include "tables.hpp"

/* #
   # HERE YOU CAN DEFINE YOUR ACTIONS
   #
   # */

inline int Drop(const struct pptags* Tags)
{
  return DROP_PACKET;
}


inline int Print(const struct pptags* Tags )
{
  std::cout << "Domain: " << Tags->Map[ DOMAIN_NAME_F ].GetString() << "\n";

  // std::cout << "M " << Tags->Metrics[0] << "\n";
  
  return CUSTOM_ACTION;
}


inline int PrintTimeoutSet(const struct pptags* Tags)
{
    std::cout << "TIMEOUT SET\n";

    return CUSTOM_ACTION;
}


inline int PrintTimeoutTrigger(const struct pptags* Tags)
{
    std::cout << "TIMEOUT TRIGGERS\n";

    return CUSTOM_ACTION;
}



inline int DoNothing(const struct pptags* Tags)
{
  const double* Met = Tags->Metrics;

  printf("Do Nothing at address: %p\n", Met);
  
  return CUSTOM_ACTION;
}


inline int Suspect(const struct pptags* Tags)
{
    Tags->Attack.Suspect = true;

    return EXPORT_PACKET;
}

// inline int print_tab(const struct pptags* Tags)
// {
//     auto ptr = tables.average_server_requests->get(Tags->Map[1]);
// 
//     std::cout << "Average Server Request: ";
//  
//     if (ptr)
//     {
//         std::cout << *ptr << "\n";
//     }
//     else
//     {
//         std::cout << "(null)\n";
//     }
// 
//     return CUSTOM_ACTION;
// }
// 
#endif


