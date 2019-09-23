cmake_minimum_required(VERSION 3.5)
project(yela)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(lib)
add_executable(yela
               src/main.cc
               src/node.cc
               src/network.cc
               src/interface.cc
               src/log.cc
               src/file_manager.cc
               src/message.cc
               src/global.cc
               src/yela.cc
               src/cmd.cc)
target_link_libraries(yela Threads::Threads)
target_link_libraries(yela ${CURSES_LIBRARIES})
target_link_libraries(yela crypto)
