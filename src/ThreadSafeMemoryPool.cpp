#include "../include/ThreadSafeMemoryPool.h"

#include <new>

template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::ThreadSafeMemoryPool()
{
    for (std::size_t i = 0; i < N; ++i) {
        free_list_.push_back(reinterpret_cast<T*>(&buffer_[i]));
    }
}

template<typename T, std::size_t N>
std::optional<T*> ThreadSafeMemoryPool<T, N>::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (free_list_.empty()) {
        return std::nullopt;
    }
    T* ptr = free_list_.back();
    free_list_.pop_back();
    // Construct object in-place using placement new
    new (ptr) T();
    return ptr;
}


template<typename T, std::size_t N>
void ThreadSafeMemoryPool<T, N>::release(T* ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (ptr) {
        ptr->~T(); // Call destructor
        free_list_.push_back(ptr);
    }
}

template<typename T, std::size_t N>
std::size_t ThreadSafeMemoryPool<T, N>::available() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return free_list_.size();
}

// Handle definitions
template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::Handle::Handle() noexcept : pool_(nullptr), ptr_(nullptr) {}

template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::Handle::Handle(ThreadSafeMemoryPool<T, N>* pool, T* ptr) noexcept : pool_(pool), ptr_(ptr) {}

template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::Handle::Handle(typename ThreadSafeMemoryPool<T, N>::Handle&& other) noexcept : pool_(other.pool_), ptr_(other.ptr_) { other.pool_ = nullptr; other.ptr_ = nullptr; }

template<typename T, std::size_t N>
typename ThreadSafeMemoryPool<T, N>::Handle& ThreadSafeMemoryPool<T, N>::Handle::operator=(typename ThreadSafeMemoryPool<T, N>::Handle&& other) noexcept {
    if (this != &other) {
        reset();
        pool_ = other.pool_; ptr_ = other.ptr_;
        other.pool_ = nullptr; other.ptr_ = nullptr;
    }
    return *this;
}

template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::Handle::~Handle() { reset(); }

template<typename T, std::size_t N>
T* ThreadSafeMemoryPool<T, N>::Handle::operator->() noexcept { return ptr_; }

template<typename T, std::size_t N>
T& ThreadSafeMemoryPool<T, N>::Handle::operator*() noexcept { return *ptr_; }

template<typename T, std::size_t N>
ThreadSafeMemoryPool<T, N>::Handle::operator bool() const noexcept { return ptr_ != nullptr; }

template<typename T, std::size_t N>
void ThreadSafeMemoryPool<T, N>::Handle::reset() noexcept {
    if (pool_ && ptr_) {
        pool_->release(ptr_);
    }
    pool_ = nullptr; ptr_ = nullptr;
}

// Explicit instantiation for Message with PoolSize 10 (used in main.cpp)
template class ThreadSafeMemoryPool<Message, 10>;