#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <mutex>



#include "safe_queue.h"


class ThreadPool
{
public:
	ThreadPool(int thread_num);
	void submit(std::function<void()> func);
	void work();
	~ThreadPool();
private:
	std::vector<std::thread> threads;
	SafeQueue<std::function<void()>> tasks;
	bool done;
};

