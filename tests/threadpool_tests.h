#pragma once

#include "../src/threadpool.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>


inline std::string string_test() {
    return "Hello world!";
}

inline int int_test(int input1, int input2) {
    return input1 + input2;
}




inline void threadpool_tests() {

    // Submit syntax
    {
        threadpool tp{1};
        auto future = tp.submit([]() {});
        tp.shutdown();
    }

    // Verify work is done on a submit
    {
        threadpool tp{1};
        int i = 0;
        auto future = tp.submit([&i](){i = 42;});
        tp.shutdown();
        assert(i == 42);
    }

    // Return type syntax
    {
        threadpool tp{1};
        auto future = tp.submit([]() {return 42;});
        tp.shutdown();
        int work = future.get();
        assert(work == 42);
    }

    // Variadic arguments works
    {
        threadpool tp{1};
        auto future = tp.submit([](int num1, int num2, int num3) {return num1 + num2 + num3;}, 1, 2, 3);
        tp.shutdown();
        assert(future.valid() && future.get() == 6);
    }

    // Function pointer works
    {
        threadpool tp{2};
        auto future1 = tp.submit(string_test);
        auto future2 = tp.submit(int_test, 1, 2);
        tp.shutdown();
        assert(future1.valid() && future1.get() == "Hello world!");
        assert(future2.valid() && future2.get() == 3);

    }

    // Function pointers with variadic arguments
    {
        threadpool tp{1};
        //std::function<int()> f1 = []() -> int {return 5;};
        std::future<int> future = tp.submit([]() {return 5;});
        tp.shutdown();
        assert(future.get() == 5);
    }

    // Ensure shutdown finishes all remaining tasks
    {
        threadpool tp{1};
        int i{0};
        auto f1 = []() { std::this_thread::sleep_for(std::chrono::milliseconds(100));};
        auto f2 = [&i]() mutable{ i = 5; };

        auto rv1 = tp.submit(f1);
        auto rv2 = tp.submit(f2);
        tp.shutdown();

        assert(i == 5);
    }


    // Submit after shutdown throws
    {
        threadpool tp{1};
        auto future = tp.submit([]() {return 42;});
        tp.shutdown();

        try {
            auto rv = tp.submit([]() {});
            assert(false);
        } catch (std::runtime_error) {

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
        auto rv1 = tp.submit([&]{

            try {
                auto rv2 = tp.submit([](){ /* work */ });
                assert(false);
            } catch (std::runtime_error) {

            }
        });
        tp.shutdown();
    }

    // get() called after shutdown_now called
    {
        threadpool tp{1};
        auto rv1 = tp.submit([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1000)); return 5; });
        auto rv2 = tp.submit([]{ std::this_thread::sleep_for(std::chrono::milliseconds(500)); return 55; });
        auto rv3 = tp.submit( []() { return 55;});

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        tp.shutdown_now();

        assert(rv1.valid() && rv1.get() == 5);
        try {
            auto error = rv3.get();
            assert(false);
        } catch (const std::future_error& e) {

        }

    }




    std::cout << "threadpool tests passed!\n";
}