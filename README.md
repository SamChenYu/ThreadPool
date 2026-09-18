# C++ Thread Pool

## Example Usage (Multiple ThreadPool Task Dependencies)
``` c++
threadpool io_pool(3); // IO tasks will be blocking
threadpool cpu_pool(2);

auto read_rv = io_pool.submit([]() { return read(); });

auto parse_rv = cpu_pool.submit([](IO_Data data) { return parse(data); }, read_rv.get());

auto compress_rv = cpu_pool.submit([](IO_Data data) { return compress(data); }, parse_rv.get());

auto upload_rv = io_pool.submit([](IO_Data data) { upload(data); }, compress_rv.get());

upload_rv.get();

io_pool.shutdown();
cpu_pool.shutdown();
```

## Example Usage (Dependency Jobs)
```c++
threadpool tp(2);

// Simulated ETL pipeline
auto api_a = tp.submit([] { return fetch_api_a(); });
auto api_b = tp.submit([] { return fetch_api_b(); });

auto clean_a = tp.submit([](data d) {
    return clean_api_a(d);
}, api_a.get());

auto clean_b = tp.submit([](data d) {
    return clean_api_b(d);
}, api_b.get());

auto merge = tp.submit([](data a, data b) {
    return merge_data(a, b);
}, clean_a.get(), clean_b.get());

auto analysis = tp.submit([](data m) {
    return run_analysis(m);
}, merge.get());

analysis.get();

tp.shutdown();
```


## Example Usage (No Dependency Jobs)
```c++
threadpool tp(4);

std::vector<std::future<int>> futures;
futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(10);} ) );
futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(20);} ) );
futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(30);} ) );
futures.emplace_back( tp.submit( []() -> int { return recursive_fibonacci(40);} ) );

tp.shutdown();

for (int i=0; i<futures.size(); i++) {
    auto& f = futures[i];
    if (f.valid()) {
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

## In Progress (Task Graph)
`taskgraph` (`src/taskgraph.hpp`) WIP. It will register task dependencies and use a topological sort to schedule them 
without blocking on `get()`.

### Planned Example Usage
```c++
taskgraph tg(2);

auto api_a = tg.enqueue([] { return fetch_api_a(); });
auto api_b = tg.enqueue([] { return fetch_api_b(); });

auto clean_a = tg.enqueue([](data d) {
    return clean_api_a(d);
}, api_a);

auto clean_b = tg.enqueue([](data d) {
    return clean_api_b(d);
}, api_b);

auto merge = tg.enqueue([](data a, data b) {
    return merge_data(a, b);
}, clean_a, clean_b);

auto analysis = tg.enqueue([](data m) {
    return run_analysis(m);
}, merge);

tg.start();

analysis.get();
```
