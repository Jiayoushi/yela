cmake_minimum_required(VERSION 3.5)
project(yela)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(yela
               src/main.cc
               src/node.cc)
