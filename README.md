# C++ Thread Pool

## Example Usage (Dependency Jobs)
```c++
threadpool tp(2);

// Simulated ETL pipeline
auto api_a = tp.submit<data>([] { return fetch_api_a(); });
auto api_b = tp.submit<data>([] { return fetch_api_b(); });

auto clean_a = api_a.then(tp, [](data d) {
    return clean_api_a(d);
});

auto clean_b = api_b.then(tp, [](data d) {
    return clean_api_b(d);
});

auto merge = tp.when_all(clean_a, clean_b)
               .then(tp, [](data a, data b) {
                   return merge_data(a, b);
               });

auto analysis = merge.then(tp, [](data m) {
    return run_analysis(m);
});
```


## Example Usage (No Dependency Jobs)
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