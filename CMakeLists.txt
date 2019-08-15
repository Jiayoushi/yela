cmake_minimum_required(VERSION 3.5)
project(yela)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
include_directories(lib)
add_executable(yela
               src/main.cc
               src/node.cc
               src/network.cc
               src/interface.cc
               src/log.cc)
target_link_libraries(yela Threads::Threads)
