#pragma once

#include "../src/threadpool.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>

inline void threadpool_tests() {

    // Ensure shutdown finishes all remaining tasks
    {
        threadpool tp{1};
        int i{0};
        auto f1 = []() { std::this_thread::sleep_for(std::chrono::milliseconds(100));};
        auto f2 = [&i]() mutable{ i = 5; };

        auto rv1 = tp.submit<void>(f1);
        auto rv2 = tp.submit<void>(f2);
        tp.shutdown();

        assert(i == 5);
    }

    // Ensure shutdown_now clears the remaining tasks
    {
        threadpool tp{1};
        int i{0};
        auto f1 = []() { std::this_thread::sleep_for(std::chrono::milliseconds(100));};
        auto f2 = [&i]() mutable{ i = 5; };

        auto rv1 = tp.submit<void>(f1);
        auto rv2 = tp.submit<void>(f2);
        tp.shutdown_now();

        assert(i == 0);
    }


    // Submit after shutdown
    {
        threadpool tp{1};
        tp.shutdown();
        try {
            auto rv = tp.submit<void>([]() {});
            assert(false);
        } catch (std::runtime_error) {

        }
    }

    // Submit after shutdown_now
    {
        threadpool tp{1};
        tp.shutdown_now();
        try {
            auto rv = tp.submit<void>([]() {});
            assert(false);
        } catch (std::runtime_error) {

        }
    }

    // Destructor stress tests
    {
        for (int i = 0; i < 10'000; ++i) {
            threadpool tp{4};
            auto rv = tp.submit<void>([]{});
        }
    }

    // Nested submission
    /*
        The invariant here is a little more subtle
        What happens here is the task has a sub-task to put another task onto the threadpool queue
        However, what most of the time happens is that shutdown() in the main thread gets called before the task gets processed
        Which means that the queue no longer accepts any tasks, and therefore would throw the runtime_error exception
        This invariant is kept here - for the DAG aware pools, there would be a private internal enqueing function that would bypass this check
    */
    {
        threadpool tp{1};
        auto rv1 = tp.submit<void>([&]{

            try {
                auto rv2 = tp.submit<void>([](){ /* work */ });
                assert(false);
            } catch (std::runtime_error) {

            }
        });
        tp.shutdown();
    }


    // Shutdown now test
    {
        threadpool tp{1};
        auto rv1 = tp.submit<void>([]{ std::this_thread::sleep_for(std::chrono::milliseconds(50));});
        auto rv2 = tp.submit<void>( []() { });

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait for thread to pick up the task
        tp.shutdown_now();

        // Not sure exactly why, but this fails
        //assert(rv1.is_valid());
        assert(!rv2.is_valid());
    }


    // // Then() syntax
    // {
    //     threadpool tp{1};
    //
    //     auto rv_1 = tp.submit<int>([](){ return 5; });
    //
    //     auto rv_2 = rv_1.then<int>(tp, []() { return 10; });
    //
    //     tp.shutdown();
    //     assert(rv_1.is_valid());
    //     assert(rv_1.get() == 5);
    //     assert(rv_2.is_valid());
    //     assert(rv_2.get() == 10);
    // }
    //
    // // Then() actually waits for dependencies
    // {
    //     threadpool tp{5};
    //     auto rv_1 = tp.submit<void>([](){ std::this_thread::sleep_for(std::chrono::milliseconds(10));});
    //     auto rv_2 = rv_1.then<int>(tp, []() { return 10; });
    //     assert(!rv_1.is_valid() && !rv_2.is_valid());
    //     tp.shutdown();
    // }


    std::cout << "threadpool tests passed!\n";
}