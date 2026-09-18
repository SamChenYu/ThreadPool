cmake_minimum_required(VERSION 4.0.2)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 23)

add_library(threadpool INTERFACE
        src/threadpool.hpp
        src/taskgraph.hpp
)
target_include_directories(threadpool INTERFACE src)
target_compile_features(threadpool INTERFACE cxx_std_23)

add_executable(demos demo/main.cpp
        tests/all_tests.hpp
        tests/threadpool_tests.hpp
        demo/dependency_demo.hpp
        demo/demo.hpp
        demo/multiple_threadpool_demo.hpp
)

add_executable(taskgraph demo/taskgraph.cpp)

target_link_libraries(taskgraph PRIVATE threadpool)

target_link_libraries(demos PRIVATE threadpool)
