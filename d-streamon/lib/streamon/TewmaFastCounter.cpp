

#include "TewmaFastCounter.hpp"

double TewmaFastCounter::add(IReadBuffer& key, double quantity, uint64_t args)
{
    tewma_node* node = getActualizedNode(key, args);

    // update value
    node->value += (quantity * -logbeta);

    return node->value;
}


double TewmaFastCounter::get(IReadBuffer& key, uint64_t args)
{
    return getActualizedNode(key, args)->value;
}

void TewmaFastCounter::clear()
{
    throw;
}
