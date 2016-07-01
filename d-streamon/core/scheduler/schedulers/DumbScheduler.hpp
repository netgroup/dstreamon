#ifndef __DUMMM_SCHED__
#define __DUMMM_SCHED__

namespace bm
{

#include<Block.hpp>
#include<MpMcQueue.hpp>

class DumbScheduler
{
	MpMcQueue<Block> m_queue;
	DumbScheduler(const DumbScheduler&)=delete;
	DumbScheduler& operator = (const DumbScheduler&)=delete;

	public:
	DumbScheduler(unsigned int) : m_queue()
	{}

	void register_thread(int, const std::vector<unsigned int>&)
	{
		//no need to register
	}

	void unregister_thread(int)
	{
		//nothing to do
	}

	void add_block(std::shared_ptr<Block>&& b)
	{
		m_queue.push(std::move(b));

	}

	void remove_block(const Block* b)
	{
		m_queue.remove(*b);
	}	

	std::shared_ptr<Block> next_task(int)
	{
		return m_queue.pop();
	}


	void task_done(int, std::shared_ptr<Block>&& b)
	{
		m_queue.push(std::move(b));
	}
};
}
#endif
