#pragma once

#include <atomic>
#include <vector>
#include <algorithm>
#include <memory>

template<typename T, typename Compare = std::less<T>>
class LockFreePriorityQueue {
private:
    struct Node {
        T value;
        std::atomic<Node*> next;
        Node(const T& val) : value(val), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<size_t> count;
    Compare comp;

public:
    LockFreePriorityQueue() : head(nullptr), count(0) {}

    void push(const T& value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load(std::memory_order_relaxed);
        do {
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
        count.fetch_add(1, std::memory_order_relaxed);
    }

    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_relaxed);
        Node* new_head;
        do {
            if (old_head == nullptr) {
                return false;
            }
            new_head = old_head->next;
        } while (!head.compare_exchange_weak(old_head, new_head,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
        result = old_head->value;
        delete old_head;
        count.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }

    bool empty() const {
        return head.load(std::memory_order_relaxed) == nullptr;
    }

    size_t size() const {
        return count.load(std::memory_order_relaxed);
    }
};
