#ifndef __RRR_SCHED__
#define __RRR_SCHED__

namespace bm
{

#include<Block.hpp>
#include<vector>
#include<shared_queue.hpp>

class RRScheduler
{
	unsigned int m_last_id;
	const unsigned int m_nq;
	std::vector<std::shared_ptr<more::shared_queue<std::shared_ptr<Block>,32 > > > m_queues;
	RRScheduler(const RRScheduler&)=delete;
	RRScheduler& operator = (const RRScheduler&)=delete;

	public:
	RRScheduler(unsigned int nq):m_last_id(0),m_nq(nq), m_queues(nq)
	{
		auto end=m_queues.end();
		for (auto it=m_queues.begin();it!=end;++it)
			*it=std::shared_ptr<more::shared_queue<std::shared_ptr<Block>,32 > >(new more::shared_queue<std::shared_ptr<Block>,32 >);
	}

	void register_thread(int id, const std::vector<unsigned int>& )
	{
		if ((unsigned int)id>=m_nq)
			throw std::runtime_error("RRSCheduler::thread index out of range" );
	}

	void unregister_thread(int)
	{
		//nothing to do
	}

	void add_block(std::shared_ptr<Block>&& b )
	{
		if(!m_queues[m_last_id++%m_nq]->push_back(std::move(b)))
			throw std::runtime_error("RRScheduler:: task queue is full");

	}

	void remove_block(const Block* b)
	{
		auto end=m_queues.end();
		for (auto it=m_queues.begin(); it!=end; ++it)
		{
			int size=(*it)->size();
			for (int i=0; i<size; ++i)
			{
				std::shared_ptr<Block> tmp;
				if(!((*it)->pop_front(tmp)))
					throw std::runtime_error("RRScheduler:: wrong size");
				if(tmp.get()==b)
					return;//block out of the queue
			(*it)->push_back(std::move(tmp));
			}
		}

		throw std::runtime_error("RRScheduler:: block to be removed was not found");
	}

	std::shared_ptr<Block> next_task(int id)
	{
		if ((unsigned int)id>=m_nq)
			throw std::runtime_error("RRSCheduler::thread index out of range" );
		
		std::shared_ptr<Block> tmp;
		if(!m_queues[id]->pop_front(tmp))
			std::cout<<"RRScheduler::WARNING:: thread number "<<id<<" has no task to execute\n";
		return tmp;
	}


	void task_done(int id, std::shared_ptr<Block>&& b)
	{
		if ((unsigned int)id>=m_nq)
			throw std::runtime_error("RRSCheduler::thread index out of range" );
		if(!m_queues[id]->push_back(std::move(b)))
			throw std::runtime_error("RRScheduler:: after taks execution task queue has become full");
	}
};
}
#endif
