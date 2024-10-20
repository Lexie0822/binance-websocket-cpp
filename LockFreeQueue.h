#pragma once

#include <atomic>
#include <memory>

template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<size_t> size_;

public:
    LockFreeQueue() : size_(0) {
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
    }

    ~LockFreeQueue() {
        while (Node* old_head = head_.load()) {
            head_.store(old_head->next);
            delete old_head;
        }
    }

    void push(T item) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(item)));
        Node* new_node = new Node();
        new_node->data = new_data;

        while (true) {
            Node* old_tail = tail_.load();
            Node* next = old_tail->next.load();

            if (old_tail == tail_.load()) {
                if (next == nullptr) {
                    if (old_tail->next.compare_exchange_weak(next, new_node)) {
                        tail_.compare_exchange_weak(old_tail, new_node);
                        size_.fetch_add(1, std::memory_order_relaxed);
                        return;
                    }
                } else {
                    tail_.compare_exchange_weak(old_tail, next);
                }
            }
        }
    }

    bool pop(T& item) {
        while (true) {
            Node* old_head = head_.load();
            Node* old_tail = tail_.load();
            Node* next = old_head->next.load();

            if (old_head == head_.load()) {
                if (old_head == old_tail) {
                    if (next == nullptr) {
                        return false;
                    }
                    tail_.compare_exchange_weak(old_tail, next);
                } else {
                    if (next) {
                        item = std::move(*next->data);
                        if (head_.compare_exchange_weak(old_head, next)) {
                            size_.fetch_sub(1, std::memory_order_relaxed);
                            delete old_head;
                            return true;
                        }
                    }
                }
            }
        }
    }

    size_t size() const {
        return size_.load(std::memory_order_relaxed);
    }
};
