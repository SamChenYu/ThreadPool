#include "../tests/all_tests.h"
#include "demo.h"
#include "dependency_demo.h"

int main() {

    all_tests();
    fibonacci_example();
    dependency_dag_example();

    return 0;
}