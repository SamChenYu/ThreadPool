#pragma once

#include "threadpool_tests.hpp"

inline void all_tests() {

    // Use the threadpools lol

    threadpool tp(1);

    auto rv3 = tp.submit(threadpool_tests);

    tp.shutdown();
}