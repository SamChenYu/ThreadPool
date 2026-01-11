#pragma once

#include "../src/threadpool.h"
#include <iostream>
#include <cassert>

inline void task_tests() {

    // Task operator() overload
    {
        int num{0};
        std::function<void()> ptr{
            [&num]() mutable { num += 5; }
        };
        task t{ptr};
        t();

        assert(num == 5);
    }

    std::cout << "task test passed!" << std::endl;
}