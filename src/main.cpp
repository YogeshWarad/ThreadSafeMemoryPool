#include "FileGuard.h"
#include "../include/ThreadSafeMemoryPool.h"

FileGuard::FileGuard(const std::string& filename, const char* mode) {
    file = std::fopen(filename.c_str(), mode);
    if (!file) throw std::runtime_error("Failed to open file");
    std::cout << "File opened: " << filename << "\n";
}

FileGuard::FileGuard(FileGuard&& other) noexcept : file(other.file) {
    other.file = nullptr;
    std::cout << "FileGuard moved (constructor)\n";
}

FileGuard& FileGuard::operator=(FileGuard&& other) noexcept {
    if (this != &other) {
        close();
        file = other.file;
        other.file = nullptr;
        std::cout << "FileGuard moved (assignment)\n";
    }
    return *this;
}

FileGuard::~FileGuard() {
    close();
}

void FileGuard::close() {
    if (file) {
        std::fclose(file);
        std::cout << "File closed\n";
        file = nullptr;
    }
}

FileGuard createFileGuard() {
    FileGuard fg("example.txt", "w");
    return fg; // Move
}

// Example usage
constexpr size_t POOL_SIZE = 10;
ThreadSafeMemoryPool<Message, POOL_SIZE> messagePool;

void worker(int thread_id) {
    for (int i = 0; i < 5; ++i) {
        auto handleOpt = messagePool.acquire_handle(i, "Hello from thread " + std::to_string(thread_id));
        if (handleOpt) {
            auto handle = std::move(*handleOpt);
            std::cout << "[Thread " << thread_id << "] Using object: " << handle->text << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            // handle will release on destruction
        } else {
            std::cout << "[Thread " << thread_id << "] Pool exhausted\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i)
        threads.emplace_back(worker, i);

    for (auto& t : threads)
        t.join();

    std::cout << "Available objects after work: " << messagePool.available() << "\n";
}