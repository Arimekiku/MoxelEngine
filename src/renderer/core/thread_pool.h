#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace Moxel
{
	class ThreadPool
	{
	public:
		ThreadPool(size_t threadsNumber = std::thread::hardware_concurrency());
		~ThreadPool();

		void enqueue(std::function<void()> task);

	private:
		std::vector<std::thread> m_threads;
		bool m_isRunning = true;

		std::queue<std::function<void()>> m_queue;
		std::mutex m_queueMutex;

		std::condition_variable m_notifier;
	};
}
