# C++ Thread Pool

## Example Usage
```c++
threadpool tp(4);

std::vector<return_value_handle<int>> futures = {
    tp.submit<int>( []() -> int { return recursive_fibonacci(10);} ),
    tp.submit<int>( []() -> int { return recursive_fibonacci(20);} ),
    tp.submit<int>( []() -> int { return recursive_fibonacci(30);} ),
    tp.submit<int>( []() -> int { return recursive_fibonacci(40);} ),
};

tp.shutdown();

for (int i=0; i<futures.size(); i++) {
    const auto& f = futures[i];
    if (f.is_valid()) {
        std::cout << "Result " << i << " " << f.get() << std::endl;
    } else {
        std::cout << "Result " << i << " not available" << std::endl;
    }
}

/*  OUTPUT
    Result 0 55
    Result 1 6765
    Result 2 832040
    Result 3 102334155
 */
```