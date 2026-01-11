#pragma once

#include "../src/threadpool.h"
#include <iostream>
#include <cassert>

inline void return_value_tests() {

    // Handle should not be valid after creating it immediately
    {
        return_value_handle<int> rv_handle1{};
        assert(!rv_handle1.is_valid());
        return_value_handle<void> rv_handle2{};
        assert(!rv_handle2.is_valid());
    }

    // Catch exception when get is invalid
    {
        return_value_handle<int> rv_handle1{};
        assert(!rv_handle1.is_valid());
        try {
            auto val = rv_handle1.get();
            assert(false);
        } catch (std::runtime_error& e) {

        }
        assert(!rv_handle1.is_valid());


        return_value_handle<void> rv_handle2{};
        assert(!rv_handle2.is_valid());
        try {
            rv_handle2.get();
            assert(false);
        } catch (std::runtime_error& e) {

        }
        assert(!rv_handle2.is_valid());
    }

    std::cout << "return_value & return_value_handle tests passed!" << std::endl;
}
