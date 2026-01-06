#pragma once

#include "return_value_tests.h"
#include "task_tests.h"
#include "threadpool_tests.h"

inline void all_tests() {
    task_tests();
    return_value_tests();
    threadpool_tests();
}