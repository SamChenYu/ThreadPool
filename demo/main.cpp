#include "../src/threadpool.h"
#include <iostream>

int recursive_fibonacci(int n) {
    if (n <= 1){
        return n;
    }
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}


int main() {
    threadpool tp(5);

     auto t1 = tp.add_task<int>( []() {recursive_fibonacci(10);} );
     auto t2 = tp.add_task<int>( []() {recursive_fibonacci(20);} );
     auto t3 = tp.add_task<int>( []() {recursive_fibonacci(30);} );
     auto t4 = tp.add_task<int>( []() {recursive_fibonacci(40);} );

    std::string output = "Queue size : " + std::to_string(tp.queue_size()) + '\n';
    std::cout << output;

    int t1_result = t1.get();

    tp.shutdown();
    std::cout << "Shut down " << std::endl;

    return 0;
}