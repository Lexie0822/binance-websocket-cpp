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
        // task has been removed from the queue,no need deleted
    }
}

void EventLoop::run() {
    thread_ = std::thread([this]() {
        while (running_) {
            io_context_.restart();  // Restart the io_context to handle new work immediately
            io_context_.run_one();  // Run a single task from the io_context (non-blocking)

            process_tasks();  // Process tasks from the queue
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
    if (event_loops_.empty()) {
        throw std::runtime_error("No event loops available in EventLoopPool.");
    }

    // Use a simple load-balancing mechanism to find the event loop with the least work
    size_t min_tasks = std::numeric_limits<size_t>::max();
    EventLoop* selected_loop = nullptr;

    for (auto& loop : event_loops_) {
        size_t task_count = loop->get_task_count();  // Assuming we add a method to get current task count
        if (task_count < min_tasks) {
            min_tasks = task_count;
            selected_loop = loop.get();
        }
    }

    // Ensure selected_loop is not null before dereferencing
    if (selected_loop == nullptr) {
        throw std::runtime_error("Failed to find a valid event loop.");
    }

    return *selected_loop;
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
