#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <chrono>
#include <optional>

template <typename T, size_t PoolSize = 1024>
class ThreadSafeMemoryPool {
    std::array<std::byte, sizeof(T) * PoolSize > buffer_;
    std:vector<T*> free_list_;
    std::mutex mutex_;
    public :
    ThreadSafeMemoryPool();
    ThreadSafeMemoryPool(const ThreadSafeMemoryPool&) = delete;
    ThreadSafeMemoryPool& operator=(const ThreadSafeMemoryPool&) = delete;
    std::optional<T*> acquire();
    void release(T* ptr);
    size_t available() const;
   };

   struct  Message
   {
    int id;
    std::string text;
    Message():id(0), text("") {}
    Message(int i, const std::string& t) : id(i), text(t) {}
   };