cmake_minimum_required(VERSION 4.0.2)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 23)

add_library(threadpool SHARED
        src/threadpool.h
        src/threadpool.cpp
        src/taskgraph.h
        src/taskgraph.cpp
)
target_include_directories(threadpool PUBLIC src)

# MSVC/MinGW don't export symbols from a DLL by default; this exports
# everything instead of hand-annotating with __declspec(dllexport/dllimport).
set_target_properties(threadpool PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(demos demo/main.cpp
        tests/all_tests.h
        tests/threadpool_tests.h
        demo/dependency_demo.h
        demo/demo.h
        demo/multiple_threadpool_demo.h
)
target_link_libraries(demos PRIVATE threadpool)
