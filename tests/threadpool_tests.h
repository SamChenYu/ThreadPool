#pragma once

#include "../src/threadpool.h"
#include <iostream>
#include <chrono>
#include <thread>

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
    std::cout << "threadpool tests passed!" << std::endl;
}