# Thread Pool Docs


## API Docs
threadpool
``` c++
// Constructor for n threads
threadpool::threadpool(const int& threads);

// Adding a task to the queue. Returns a handle to give return value of the function pointer.
std::future<T> threadpool::submit(const std::function<T()>& ptr);

// Block the queue, join all threads (All pending tasks run).
void threadpool::shutdown();

// Block and clear the queue, join all threads (All pending tasks cleared. Current tasks on threads run.).
void threadpool::shutdown_now();

// Number of tasks awaiting in the queue. Does not count tasks in progress. 
size_t threadpool::queue_size() const;
``` 

## Pool Main Thread vs Worker Thread
Worker Thread Logic
```
WHILE TRUE:
    Acquire Mutex
    
    WHILE queue.empty() && !stop
        Sleep until notified // cv.wait(lock) 
        // Spurious wakeups
    
    If queue.empty() && stop
        Release mutex
        exit()
    
    
    Poll the queue
    Release mutex
    
    Execute task
```

Thread Pool Logic
```
Init n threads
Await for task submit() or shutdown()

Function submit(task)
    Acquire mutex
    If stop == true
        Release mutex
        throw runtime error
    Add task to queue
    Release mutex
    Notify one thread
    
Function shutdown()
    Acquire mutex
    stop = true
    Release mutex
    notifyall()
    For each threads
        thread.join()
    
    
Function shutdown_now()
    Acquire mutex
    stop = true
    Clear queue
    Release mutex
    notifyall()
    For each threads
        thread.join()
```


# Variadic Templating
```c++
template<typename Function, typename... Args>
[[nodiscard]]
auto submit(Function &&F, Args &&...ArgList) {

    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    if (m_Stop) {
        throw std::runtime_error{"ThreadPool::submit() after shutdown called"};
    }


    using ReturnType = std::invoke_result_t<Function, Args...>;

    std::shared_ptr<std::packaged_task<ReturnType()>> task = std::make_shared<std::packaged_task<ReturnType()>>((
        std::bind(std::forward<Function>(F),
                  std::forward<Args>(ArgList)...)
    ));

    auto future = task->get_future();

    write_task([task](){ (*task)(); });

    return future; // Return type is future<ReturnType>
}
```





## DAG Dependency APIs
- Important points: The threadpool needs to own the tasks, even ones with dependencies (Allow cross-threadpool dependencies(?))
### Conceptual Models - singular dependency
```c++
auto api_a = tp.submit<data>([] { return fetch_api_a(); });

auto clean_a = api_a.then(tp, [](data d) {
    return clean_api_a(d);
});
```



## Continuation
Hybrid design
- A thread pool that executes tasks
- A task graph builder that:
  - collects nodes
  - collects dependencies
  - validates acyclicity
  - topo-sorts

Then submits runnable nodes to the pool

```c++
task_graph g;

auto a = g.add(fetch);
auto b = g.add(clean).after(a);
auto c = g.add(merge).after(a, b);

g.execute(tp);
```

Internally:
topo sort once
decrement dependency counters
enqueue ready tasks
on completion, unlock children
