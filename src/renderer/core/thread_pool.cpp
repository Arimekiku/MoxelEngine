#include "thread_pool.h"

namespace Moxel
{
	ThreadPool::ThreadPool(const size_t threadsNumber)
	{
		using namespace std;

		for (size_t i = 0; i < threadsNumber; i++)
		{
			m_threads.emplace_back([this]
			{
				while (true)
				{
					unique_lock lock(m_queueMutex);
					m_notifier.wait(lock, [this]
					{
						return m_queue.empty() == false || is_running == false;
					});

					if (is_running == false)
					{
						return;
					}

					function<void()> task = std::move(m_queue.front());
					m_queue.pop();

					task();
				}
			});
		}
	}

	ThreadPool::~ThreadPool()
	{
		std::unique_lock lock = std::unique_lock(m_queueMutex);
		is_running = false;
		lock.unlock();

		m_notifier.notify_all();
		for (auto& thread : m_threads)
		{
			thread.join();
		}
	}

	void ThreadPool::enqueue(std::function<void()> task)
	{
		std::unique_lock lock(m_queueMutex);
		m_queue.emplace(std::move(task));

		m_notifier.notify_one();
	}
}