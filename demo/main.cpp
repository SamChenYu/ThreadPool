#include "../src/threadpool.h"
#include <iostream>


#include <chrono>   // Required for std::chrono
#include <thread>   // Required for std::this_thread

int recursive_fibonacci(int n) {
    if (n <= 1){
        return n;
    }
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}


int main() {
    threadpool tp(1);

    auto t1 = tp.submit<int>( []() -> int { return recursive_fibonacci(10);} );
     auto t2 = tp.submit<int>( []() -> int {return recursive_fibonacci(20);} );
     auto t3 = tp.submit<int>( []() -> int {return recursive_fibonacci(30);} );
     auto t4 = tp.submit<int>( []() -> int { return recursive_fibonacci(40);} );

    // std::string output = "Queue size : " + std::to_string(tp.queue_size()) + '\n';
    // std::cout << output;
    std::this_thread::sleep_for(std::chrono::seconds(5));


    if (t1.is_valid()) {
        int t1_result = t1.get();
        std:: cout << t1_result << std::endl;
    } else {
        std::cout << "T1 not available " << std::endl;
    }


    tp.shutdown();
    // std::cout << "Shut down " << std::endl;

    return 0;
}