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








