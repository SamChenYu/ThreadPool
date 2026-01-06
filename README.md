# C++ Thread Pool

## Example Usage
```c++
threadpool tp(2); // Spawn 2 worker threads

auto t1 = tp.submit<int>( []() -> int { return recursive_fibonacci(10);} );
auto t2 = tp.submit<int>( []() -> int { return recursive_fibonacci(20);} );
auto t3 = tp.submit<int>( []() -> int { return recursive_fibonacci(30);} );
auto t4 = tp.submit<int>( []() -> int { return recursive_fibonacci(40);} );

if (t1.is_valid()) {
 int t1_result = t1.get();
 std:: cout << t1_result << std::endl;
} else {
 std::cout << "T1 not available " << std::endl;
}

tp.shutdown();
```




```
Todo


- Template Function into std::function<T()>
- Worker main logic (research how to not poll an atomic variable all the time, and block instead)
- Shutdown / Shutdown now

- Submit task validation 

```