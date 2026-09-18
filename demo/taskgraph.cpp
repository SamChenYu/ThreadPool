#include <iostream>
#include "../src/taskgraph.hpp"

int main() {

    taskgraph tg{5};
    auto result = tg.enqueue(
        []() {
            std::cout << "Function 1" << std::endl;
        }
    );
    return 0;
}