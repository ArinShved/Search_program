#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>



#include "safe_queue.h"


class ThreadPool
{
public:
	ThreadPool(int thread_num);
	void submit(std::function<void()> func);
	void work();
	~ThreadPool();
	void wait();

	bool is_done();
private:
	std::vector<std::thread> threads;
	SafeQueue<std::function<void()>> tasks;
	std::atomic<bool> done;
};

