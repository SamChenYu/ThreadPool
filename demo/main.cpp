#include <iostream>

#include "../src/threadpool.h"
#include "../tests/all_tests.h"

// For the time pause only
#include <chrono>
#include <thread>

int recursive_fibonacci(int n) {
    if (n <= 1){
        return n;
    }
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

void fibonacci_example() {

    threadpool tp(3);

    std::vector<return_value_handle<int>> futures = {
        tp.submit<int>( []() -> int { return recursive_fibonacci(20);} ),
        tp.submit<int>( []() -> int { return recursive_fibonacci(30);} ),
        tp.submit<int>( []() -> int { return recursive_fibonacci(40);} ),
    };

    tp.shutdown();

    for (int i=0; i<futures.size(); i++) {
        const auto& f = futures[i];
        if (f.is_valid()) {
            std::cout << "Result " << i << " " << f.get() << std::endl << std::flush;
        } else {
            std::cout << "Result " << i << " not available" << std::endl << std::flush;
        }
    }
}

struct data{};

void dependency_dag_example() {

}







int main() {

    //all_tests();
    fibonacci_example();

    return 0;
}