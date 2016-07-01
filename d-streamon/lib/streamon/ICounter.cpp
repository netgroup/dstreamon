
#include "ICounter.hpp"
#include "TewmaCounter.hpp"
#include "TewmaFastCounter.hpp"
#include "DLeftCounter.hpp"
#include "SimpleCounter.hpp"

ICounter* ICounter::getInstance(std::string type,
                                int         nhash,
                                int         shash,
                                double      beta) // only for TEWMA
{
    if (type == "tewma") return new TewmaCounter(nhash, shash, beta, 1);
    else
    if (type == "tewma-fast") return new TewmaFastCounter(nhash, shash, beta, 1);
    else
    if (type == "dleft") return new DLeftCounter(nhash, shash);
    else
    if (type == "simple") return new SimpleCounter();

    return NULL;
}
