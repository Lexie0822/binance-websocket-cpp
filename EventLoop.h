#pragma once

#include <boost/asio.hpp>
#include "LockFreePriorityQueue.h"
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

class EventLoop {
public:
    enum class Priority { Low, Medium, High };

    EventLoop();
    ~EventLoop();

    void run();
    void stop();
    void post(std::function<void()> task, Priority priority = Priority::Medium);

    boost::asio::io_context& get_io_context() { return io_context_; }
    size_t get_task_count() const { return tasks_.size(); }

private:
    struct PrioritizedTask {
        std::function<void()> task;
        Priority priority;

        bool operator<(const PrioritizedTask& other) const {
            return priority < other.priority;
        }
    };

    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    std::thread thread_;
    std::atomic<bool> running_{true};
    LockFreePriorityQueue<PrioritizedTask> tasks_;

    void process_tasks();
};

class EventLoopPool {
public:
    EventLoopPool(size_t thread_count);
    ~EventLoopPool();

    EventLoop& get_next_event_loop();
    void run();
    void stop();
    size_t size() const { return event_loops_.size(); }

private:
    std::vector<std::unique_ptr<EventLoop>> event_loops_;
    std::atomic<size_t> next_loop_{0};
};
