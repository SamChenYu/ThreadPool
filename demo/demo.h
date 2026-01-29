#pragma once

#include <vector>
#include <iostream>
#include "../src/threadpool.h"

// Simple demo usage of threadpool

inline int recursive_fibonacci(int n) {
    if (n <= 1){
        return n;
    }
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

inline void fibonacci_example() {

    threadpool tp(3);

    std::vector<std::future<int>> futures;
    futures.reserve(3);
    futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(10);}));
    futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(20);}));
    futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(30);}));
    tp.shutdown();

    for (int i=0; i<futures.size(); i++) {
        auto& f = futures[i];
        if (f.valid()) {
            std::cout << "Result " << i << " " << f.get() << std::endl << std::flush;
        } else {
            std::cout << "Result " << i << " not available" << std::endl << std::flush;
        }
    }
}