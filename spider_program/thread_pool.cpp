#include "thread_pool.h"

ThreadPool::ThreadPool(int thread_num)
{
    done = false;
    for (size_t i = 0; i < thread_num; ++i)
    {
        threads.emplace_back([this] 
            {
            this->work(); 
            });
    }
};

void ThreadPool::work() 
{
    while (!done) 
    {
        try 
        {
            std::function<void()> task = std::move(tasks.queue_pop());
            task();
        }
        catch (const std::runtime_error& e) 
        {
            if (done)
            {
                break;
            }
            continue;
        }
    }
};

ThreadPool::~ThreadPool() 
{
    stop_with_wait();
};

    
void ThreadPool::submit(std::function<void()> func) 
{
    if (done|| !func)
    {
        return;
    }
    tasks.queue_push(func);
};

void ThreadPool::stop_with_wait() 
{
    while (!tasks.queue_empty() && !done)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    done = true;
    tasks.set_done();


    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
  
bool ThreadPool::is_done()
{
    return tasks.queue_empty();
}