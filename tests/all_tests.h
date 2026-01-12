#pragma once

#include "return_value_tests.h"
#include "task_tests.h"
#include "threadpool_tests.h"

inline void all_tests() {

    // Use the threadpools lol

    threadpool tp(3);

    auto rv1 = tp.submit<void>(task_tests);
    auto rv2 = tp.submit<void>(return_value_tests);
    auto rv3 = tp.submit<void>(threadpool_tests);

    tp.shutdown();
}