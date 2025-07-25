#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

template<typename T>
class SafeQueue {
public:
	SafeQueue() : done(false) {};

	T queue_pop() {
		std::unique_lock<std::mutex> lock(mtx);
		notification.wait(lock, [this] {
			return !task_queue.empty() || done;
			});
		if (done && task_queue.empty()) {
			throw std::runtime_error("empty and done");
		}
		T data = std::move(task_queue.front());
		task_queue.pop();
		return data;
	};

	void queue_push(T task) {
		std::unique_lock<std::mutex> lock(mtx);
		task_queue.push(std::move(task));
		notification.notify_one();
	};

	bool queue_empty() {
		std::unique_lock<std::mutex> lock(mtx);
		return task_queue.empty();
	};

	void set_done() {
		std::unique_lock<std::mutex> lock(mtx);
		done = true;
		notification.notify_all();
	};

	~SafeQueue() { set_done(); }

private:
	std::queue<T> task_queue;
	std::mutex mtx;
	std::condition_variable notification;
	std::atomic<bool> done;
};

