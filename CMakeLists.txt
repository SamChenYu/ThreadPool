cmake_minimum_required(VERSION 4.0.2)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 23)

add_executable(ThreadPool demo/main.cpp
        src/threadpool.h
        src/threadpool.cpp
        tests/all_tests.h
        tests/threadpool_tests.h
        demo/dependency_demo.h
        demo/demo.h
        demo/multiple_threadpool_demo.h
        src/taskgraph.cpp
        src/taskgraph.h
)
