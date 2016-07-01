
#ifndef _SHARED_LIBRARY_H
#define _SHARED_LIBRARY_H

#include "pptags.h"

typedef double(*feature_function)(double*);

typedef bool(*condition_function)(double*);

typedef int(*action_function)(struct pptags*);


typedef struct
{
  condition_function Condition;
  action_function* Action;
  int NextState;

} condition_tuple;


typedef struct
{
   feature_function* FeatureList;
   condition_tuple* ConditionList;
   action_function* PostCondActionList;

} state_tuple;

#endif
