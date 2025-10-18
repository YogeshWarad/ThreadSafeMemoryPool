cmake_minimum_required(VERSION 3.10)
project(FileGuardProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

## Add sources for the FileGuard executable. ThreadSafeMemoryPool has an
## out-of-class implementation file with explicit instantiation (src/ThreadSafeMemoryPool.cpp).
add_executable(FileGuard
	src/main.cpp
	src/ThreadSafeMemoryPool.cpp
)
