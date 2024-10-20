#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <random>
#include "LockFreeQueue.h"

class WorkStealingThreadPool {
public:
    WorkStealingThreadPool(size_t num_threads);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~WorkStealingThreadPool();

private:
    std::vector<std::thread> workers;
    std::vector<LockFreeQueue<std::function<void()>>> queues;
    std::atomic<bool> stop;
    std::atomic<size_t> index;

    void worker_thread(size_t id);
    bool pop_task_from_local_queue(std::function<void()>& task, size_t id);
    bool pop_task_from_other_queue(std::function<void()>& task, size_t id);
};

// the constructor just launches some amount of workers
inline WorkStealingThreadPool::WorkStealingThreadPool(size_t num_threads)
    :   stop(false)
{
    for(size_t i = 0;i<num_threads;++i)
        workers.emplace_back(
            [this, i]
            {
                worker_thread(i);
            }
        );
}

// Implementation of enqueue function
template<class F, class... Args>
auto WorkStealingThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
        
    std::future<return_type> res = task->get_future();
    
    size_t i = index++;
    queues[i % queues.size()].push([task](){ (*task)(); });
    
    return res;
}

// the destructor joins all threads
inline WorkStealingThreadPool::~WorkStealingThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutexes[0]);
        stop = true;
    }
    condition_vars[0].notify_all();
    for(std::thread &worker: workers)
        worker.join();
}
