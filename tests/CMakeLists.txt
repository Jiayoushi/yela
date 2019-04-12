cmake_minimum_required(VERSION 3.5)
project(yela)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(run_tests
               test_main.cc

               rpc_unittest.cc
               ../src/rpc.cc

               raft_unittest.cc
               ../src/raft.cc)
target_link_libraries(run_tests ${GTEST_LIBRARIES} pthread)
