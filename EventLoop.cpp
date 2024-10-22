#include "EventLoop.h"
#include "LockFreePriorityQueue.h"
#include <iostream>
#include <stdexcept>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <queue>

EventLoop::EventLoop() : work_(std::make_unique<boost::asio::io_context::work>(io_context_)) {}

EventLoop::~EventLoop() {
    stop();
    PrioritizedTask task;
    while (tasks_.pop(task)) {
        // Clear remaining tasks
    }
}

void EventLoop::run() {
    thread_ = std::thread([this]() {
        while (running_) {
            io_context_.restart();
            io_context_.poll(); // Using poll to handle multiple tasks in one go (non-blocking)
            process_tasks(); // Process tasks from the queue
        }
    });
}

void EventLoop::stop() {
    running_ = false;
    work_.reset();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void EventLoop::post(std::function<void()> task, Priority priority) {
    tasks_.push({std::move(task), priority});
}

void EventLoop::process_tasks() {
    PrioritizedTask task;
    while (tasks_.pop(task)) {
        task.task();
    }
}

size_t EventLoop::get_task_count() const {
    return tasks_.size();
}

EventLoopPool::EventLoopPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        event_loops_.push_back(std::make_unique<EventLoop>());
    }
}

EventLoopPool::~EventLoopPool() {
    stop();
}

void EventLoopPool::run() {
    for (auto& loop : event_loops_) {
        loop->run();
    }
}

void EventLoopPool::stop() {
    for (auto& loop : event_loops_) {
        loop->stop();
    }
}

EventLoop& EventLoopPool::get_next_event_loop() {
    if (event_loops_.empty()) {
        throw std::runtime_error("No event loops available in EventLoopPool.");
    }
    // Improved load balancing: select event loop with the least current workload
    size_t min_tasks = std::numeric_limits<size_t>::max();
    EventLoop* selected_loop = nullptr;

    for (auto& loop : event_loops_) {
        size_t task_count = loop->get_task_count();
        if (task_count < min_tasks) {
            min_tasks = task_count;
            selected_loop = loop.get();
        }
    }

    if (selected_loop == nullptr) {
        throw std::runtime_error("Failed to find a valid event loop.");
    }

    return *selected_loop;
}

