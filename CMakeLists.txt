cmake_minimum_required(VERSION 3.2)
project (hyperbench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

add_executable(hyperbench main.cpp hayai_tests.cpp)
target_link_libraries(hyperbench Threads::Threads)

