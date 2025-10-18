#ifndef FILE_GUARD_H
#define FILE_GUARD_H

#include <iostream>
#include <string>
#include <cstdio>

class FileGuard {
private:
    FILE* file = nullptr;

public:
    explicit FileGuard(const std::string& filename, const char* mode = "r");

    // Non-copyable
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;

    // Movable
    FileGuard(FileGuard&& other) noexcept;
    FileGuard& operator=(FileGuard&& other) noexcept;

    ~FileGuard();

private:
    void close();
};

#endif // FILE_GUARD_H
