# Thread Pool Docs
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
The threads simply invoke the task. 




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








