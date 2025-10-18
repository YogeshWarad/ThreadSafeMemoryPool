#ifndef THREAD_SAFE_MEMORY_POOL_H
#define THREAD_SAFE_MEMORY_POOL_H

#include <array>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <chrono>
#include <optional>
#include <cstddef>
#include <string>
#include <iostream>
#include <type_traits>

template <typename T, std::size_t PoolSize = 1024>
class ThreadSafeMemoryPool {
public:
    ThreadSafeMemoryPool();
    ThreadSafeMemoryPool(const ThreadSafeMemoryPool&) = delete;
    ThreadSafeMemoryPool& operator=(const ThreadSafeMemoryPool&) = delete;

    std::optional<T*> acquire();
    
    // RAII handle that releases the object back to the pool when destroyed.
    class Handle {
    public:
        Handle() noexcept;
        Handle(ThreadSafeMemoryPool* pool, T* ptr) noexcept;
        Handle(Handle&& other) noexcept;
        Handle& operator=(Handle&& other) noexcept;
        ~Handle();

        T* operator->() noexcept;
        T& operator*() noexcept;
        explicit operator bool() const noexcept;

        void reset() noexcept;

        // non-copyable
        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;

    private:
        ThreadSafeMemoryPool* pool_;
        T* ptr_;
    };

    template<typename... Args>
    std::optional<Handle> acquire_handle(Args&&... args);
    void release(T* ptr);
    std::size_t available() const;

private:
    // Use aligned storage for each object slot to guarantee correct alignment
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, PoolSize> buffer_;
    std::vector<T*> free_list_;
    mutable std::mutex mutex_;
};

// Definitions are provided in src/ThreadSafeMemoryPool.cpp (explicit instantiation
// is used for the types needed by the project).

// Small example POD/struct used by the project
struct Message {
    int id;
    std::string text;
    Message(): id(0), text() {}
    Message(int i, const std::string& t) : id(i), text(t) {}
};

#endif // THREAD_SAFE_MEMORY_POOL_H

// Member-template implementation: must be visible to callers.
template<typename T, std::size_t PoolSize>
template<typename... Args>
std::optional<typename ThreadSafeMemoryPool<T, PoolSize>::Handle> ThreadSafeMemoryPool<T, PoolSize>::acquire_handle(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (free_list_.empty()) return std::nullopt;
    T* ptr = free_list_.back();
    free_list_.pop_back();
    new (ptr) T(std::forward<Args>(args)...);
    return ThreadSafeMemoryPool<T, PoolSize>::Handle(this, ptr);
}
