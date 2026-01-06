#pragma once

#include "../src/threadpool.h"
#include <iostream>

void return_value_tests() {

    return_value_handle<int> rv_handle1{};
    assert(!rv_handle1.is_valid());
    return_value_handle<void> rv_handle2{};
    assert(!rv_handle2.is_valid());

    return_value_handle<int> h1;
    auto h2 = h1; // copy
    assert(h1.is_valid() == h2.is_valid());
    // set value via h1's underlying state and check h2 sees it



    std::cout << "return_value & return_value_handle tests passed!" << std::endl;
}
