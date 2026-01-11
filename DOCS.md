# Thread Pool Docs


## API Docs
threadpool
``` c++
// Constructor for n threads
threadpool::threadpool(const int& threads);

// Adding a task to the queue. Returns a handle to give return value of the function pointer.
return_value_handle<T> threadpool::submit(const std::function<T()>& ptr);

// Block the queue, join all threads (All pending tasks run).
void threadpool::shutdown();

// Block and clear the queue, join all threads (All pending tasks cleared. Current tasks on threads run.).
void threadpool::shutdown_now();

// Number of tasks awaiting in the queue. Does not count tasks in progress. 
int threadpool::queue_size() const;
``` 

return_value_handle<T>
```c++
// Used to check if the thread has finished work and if get() is accessible
bool return_value_handle::is_valid()

// Get the return value from the function. Will throw std::runtime_error if is_valid = false 
T return_value_handle::get()
```













## `Task` vs `Return_Value` vs `Return_Value_Handle`

Data Flow
```
Client creates threadpool with n threads
Client calls submit(function<T()>);
    threadpool creates a return_value_handle
    threadpool wraps the function pointer into a task
    threadpool places the task on the queue
    threadpool returns the client the return_value_handle

Once thread is done, rv_handle.is_valid() = true
Client can obtain return value with rv_handle.get()
```
### Return_Value_Handle?
The return_value state is passed both to the client and the task, and therefore requires the handle to hold a shared_pointer. 

### Task wraps a `function<void()>`, but the client submits a `function<T()>`?
threadpool wraps the function pointer:
``` c++
[rv_handle, ptr]() { 
    rv_handle.m_Value = ptr();
    rv_handle.valid = true;
}
```
This way we are able to capture the return value to the client. 
The threads simply invoke the task. (This is simplified, we acquire mutexes to prevent race conditions)




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





## DAG Dependency APIs

### Conceptual Models - singular dependency
```c++
auto api_a = tp.submit<data>([] { return fetch_api_a(); });

auto clean_a = api_a.then(tp, [](data d) {
    return clean_api_a(d);
});
```
return_value_handle contains then():
- return_value needs to add a vector<std::function<T()> to store callback functions
- Once the return_value has been validated, it enqueues the next task onto the corresponding threadpool
``` psuedocode
then(tp, f):
    if state.ready:
        enqueue f(state.value) onto tp
    else:
        state.continuations.push_back({tp, f})
```

### Conceptual Models - multiple dependency
``` c++
auto merge = tp.when_all(clean_a, clean_b)
               .then(tp, [](data a, data b) {
                   return merge_data(a, b);
               });
```
return_value_handle contains .when_all()
- register a function callback into clean_a and clean_b 
- on completion, check if merge can fire 
- say clean_a fires first, clean_b still not done, do nothing 
- clean_b fires second, then merge's task can be enqueued

note: error propagation - my decision for any errors is to cancel the entire task completely



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
