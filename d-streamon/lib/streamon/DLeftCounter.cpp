
#include "DLeftCounter.hpp"


double DLeftCounter::get(IReadBuffer& key, uint64_t args)
{
	counterLock->lock(false);

    auto entry = table->get(key);

    if (entry == NULL)
	{
		counterLock->unlock();

		return 0;
	}

	double res = *entry;

	counterLock->unlock();

    return res;
}


double DLeftCounter::add(IReadBuffer& key, double quantity, uint64_t args)
{
	counterLock->lock(true);

    auto entry = table->get(key);

    if (entry == NULL)
    {
        entry = table->reserve(key);

        assert(entry);

        *entry = 0;
    }

    *entry += quantity;

	double res = *entry;

	counterLock->unlock();

    return res;
}

void DLeftCounter::clear()
{
	counterLock->lock(true);

    table->clear();

	counterLock->unlock();
}


bool DLeftCounter::reset(IReadBuffer& key)
{
	counterLock->lock(true);

	bool res = table->remove(key);

	counterLock->unlock();

    return res;
}
