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
        // 任务已经被移出队列，不需要删除
    }
}

void EventLoop::run() {
    thread_ = std::thread([this]() {
        while (running_) {
            io_context_.run_for(std::chrono::milliseconds(10));
            process_tasks();
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


EventLoopPool::EventLoopPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        event_loops_.push_back(std::make_unique<EventLoop>());
    }
}

EventLoopPool::~EventLoopPool() {
    stop();
}

EventLoop& EventLoopPool::get_next_event_loop() {
    return *event_loops_[next_loop_++ % event_loops_.size()];
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
