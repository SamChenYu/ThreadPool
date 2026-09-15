#include "../tests/all_tests.hpp"
#include "demo.hpp"
#include "dependency_demo.hpp"
#include "multiple_threadpool_demo.hpp"

int main() {
    all_tests();
    fibonacci_example();
    dependency_dag_example();
    multiple_threadpool_example();

    return 0;
}