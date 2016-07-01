
#include "ICounter.hpp"
#include "DLeft.cpp"
#include <cmath>

struct tewma_node
{
    time_t timestamp;
    double value;
};

class TewmaFastCounter : public ICounter
{

    DLeft<tewma_node>* data;

    // const double beta;
    const double logbeta;
    const double window;

    tewma_node* getActualizedNode(IReadBuffer& key, uint64_t timestamp)
    {
        time_t current_timestamp = (time_t) timestamp;

        tewma_node* node = data->get(key);

        if (node == NULL)
        {
            node = data->reserve(key);

            assert(node);

            node->timestamp = 0;
            node->value     = 0;
        }

        int64_t deltat = current_timestamp - node->timestamp;

        assert(deltat >= 0);

        // timestamp update
        node->timestamp = current_timestamp;
    
        // actualization
        node->value *= exp( deltat/window * logbeta);

        return node;
    }

public:

    TewmaFastCounter(int nhash, int shash, double beta, double _window) : data( new DLeft<tewma_node>(nhash, shash) ),
                                                                           // beta(_beta),
                                                                           logbeta( log(beta) ),
                                                                           window(_window)
    {
    }

    virtual double get(IReadBuffer& key, uint64_t args=0);
    virtual double add(IReadBuffer& key, double quantity=1, uint64_t args=0);

    virtual void clear();

    virtual ~TewmaFastCounter()
    {
        delete data;
    }
};
