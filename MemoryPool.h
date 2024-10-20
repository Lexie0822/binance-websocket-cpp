#pragma once

#include <cstddef>
#include <vector>
#include <mutex>
#include <memory>

template <typename T, size_t BlockSize = 4096>
class MemoryPool {
private:
    struct Block {
        char data[BlockSize];
        Block* next;
    };

    struct Chunk {
        char* data;
        Chunk* next;
    };

    Block* currentBlock_;
    Chunk* freeChunks_;
    std::ptrdiff_t chunksAvailable_;
    std::mutex mutex_;

public:
    MemoryPool() noexcept : currentBlock_(nullptr), freeChunks_(nullptr), chunksAvailable_(0) {}

    MemoryPool(const MemoryPool& other) = delete;
    MemoryPool& operator=(const MemoryPool& other) = delete;

    ~MemoryPool() noexcept {
        Block* curr = currentBlock_;
        while (curr != nullptr) {
            Block* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (freeChunks_ != nullptr) {
            T* result = reinterpret_cast<T*>(freeChunks_->data);
            freeChunks_ = freeChunks_->next;
            return result;
        }
        if (chunksAvailable_ < 1) {
            allocateBlock();
        }
        return reinterpret_cast<T*>(currentBlock_->data + (BlockSize - chunksAvailable_-- * sizeof(T)));
    }

    void deallocate(T* p) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        Chunk* chunk = reinterpret_cast<Chunk*>(p);
        chunk->next = freeChunks_;
        freeChunks_ = chunk;
    }

private:
    void allocateBlock() {
        Block* newBlock = new Block;
        newBlock->next = currentBlock_;
        currentBlock_ = newBlock;
        chunksAvailable_ = BlockSize / sizeof(T);

        char* start = newBlock->data;
        char* end = start + BlockSize;
        for (char* p = start; p + sizeof(T) <= end; p += sizeof(T)) {
            Chunk* chunk = reinterpret_cast<Chunk*>(p);
            chunk->next = freeChunks_;
            freeChunks_ = chunk;
        }
    }
};
