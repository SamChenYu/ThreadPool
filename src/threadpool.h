#pragma once
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

// Forward Declarations
template<class T>
struct return_value_handle;
class threadpool;


// Wrapper for the function pointers
struct task {
    task(const std::function<void()>& ptr) : m_Ptr {ptr} {
    };

    void operator()() const {
        m_Ptr();
    }
private:
    std::function<void()> m_Ptr;
};

// Simplified implementation of std::future
// return_value is shared state via the shared pointer, do not allow copy / move semantics
template<class T>
struct return_value {

    friend class return_value_handle<T>; // Allow access to set_value and set_valid

    return_value() = default;

    return_value(const return_value&) = delete; // Copy Constructor
    return_value& operator=(const return_value&) = delete; // Copy Assignment Constructor

    return_value(return_value&&) = delete; // Move Constructor
    return_value& operator=(return_value&&) = delete; // Move Assignment Constructor

    bool is_valid() {
        std::unique_lock<std::mutex> lock(access_mutex);
        return m_IsValid;
    }

    T get() {
        std::unique_lock<std::mutex> lock(access_mutex);
        if (!is_valid_unsafe()) { // unsafe to prevent deadlock as we already acquired mutex
            throw std::runtime_error{"Thread Return Value is Invalid!"};
        }
        m_IsValid = false;
        return m_Value;
    }

private:
    std::mutex access_mutex;
    T m_Value;
    bool m_IsValid{false};

    bool is_valid_unsafe() {
        return m_IsValid;
    }

    void set_value(const T& value) {
        std::unique_lock<std::mutex> lock(access_mutex);
        m_Value = value;
        m_IsValid = true;
        // Todo: Forward callbacks to their pools
    }


    std::vector<std::function<void()>> callbacks;
    // Dependency DAG APIs
    template<class S>
    return_value_handle<S> then(threadpool& tp, std::function<S()>& f) {
        return_value_handle<S> rv_handle{};
        // Forward to tp
        callbacks.emplace_back(
            [tp, f, rv_handle]() {
                tp.submit_internal<S>(f, rv_handle);
            }
        );
        return rv_handle;
    }



};

// Specialization of void
template<>
struct return_value<void> {

    friend class return_value_handle<void>; // Allow access to set_value and set_valid

    return_value() = default;

    return_value(const return_value&) = delete; // Copy Constructor
    return_value& operator=(const return_value&) = delete; // Copy Assignment Constructor

    return_value(return_value&&) = delete; // Move Constructor
    return_value& operator=(return_value&&) = delete; // Move Assignment Constructor

    bool is_valid() {
        std::unique_lock<std::mutex> lock(access_mutex);
        return m_IsValid;
    }

    void get() {
        std::unique_lock<std::mutex> lock(access_mutex);
        if (!is_valid_unsafe()) {
            lock.unlock();
            throw std::runtime_error{"Thread Return Value is Invalid!"};
        }
        // nothing to return
    }

    //Todo: specizalition of then()

private:
    std::mutex access_mutex;
    bool m_IsValid{false};

    bool is_valid_unsafe() {
        return m_IsValid;
    }

    void set_value() {
        std::unique_lock<std::mutex> lock(access_mutex);
        m_IsValid = false; // Never true with <void>
        // Do nothing
    }
};


template<class T>
struct return_value_handle {

    friend class threadpool;

public:
    return_value_handle() : m_Handle{std::make_shared<return_value<T>>()} {
    }

    return_value_handle(const return_value_handle&) = default; // Copy Constructor
    return_value_handle& operator=(const return_value_handle&) = default; // Copy Assignment Constructor

    return_value_handle(return_value_handle&&) = default; // Move Constructor
    return_value_handle& operator=(return_value_handle&&) = default; // Move Assignment Constructor

    bool is_valid() const {
        if (m_Handle == nullptr)
            return false;

        return m_Handle -> is_valid();
    }

    T get() const {
        return m_Handle.get()->get();
    }

    // Dependency DAG APIs
    template<class S>
    return_value_handle<T> then(threadpool& tp, std::function<S()>& f) {
        return m_Handle->then(tp, f);
    }

private:
    std::shared_ptr<return_value<T>> m_Handle;
    void set_value(const T& value) {
        m_Handle->set_value(value);
    }
    void set_valid(const bool& value) {
        m_Handle->set_valid(value);
    }
};

// Specialization of void
template<>
struct return_value_handle<void> {

    friend class threadpool;

public:
    return_value_handle() : m_Handle{std::make_shared<return_value<void>>()} {
    }

    return_value_handle(const return_value_handle&) = default; // Copy Constructor
    return_value_handle& operator=(const return_value_handle&) = default; // Copy Assignment Constructor

    return_value_handle(return_value_handle&&) = default; // Move Constructor
    return_value_handle& operator=(return_value_handle&&) = default; // Move Assignment Constructor

    bool is_valid() const {
        if (m_Handle == nullptr)
            return false;

        return m_Handle -> is_valid();
    }

    void get() const {
        return m_Handle.get() -> get();
    }

    // Todo: specialization of then()

private:
    std::shared_ptr<return_value<void>> m_Handle;
    static void set_value() {
        // Do nothing
    }
};











class threadpool {

    template<typename> friend struct return_value; // Give access to submit_internal

private:
    std::queue<task> tasks;
    std::vector<std::thread> workers;

    bool m_Stop{false};
    std::mutex queue_stop_mutex; // Used for queue operations and read/write m_Stop operations
    std::condition_variable cv;


    std::optional<task> poll_task();
    void write_task(const std::function<void()>& ptr) {
        // No lock guard as submit() already contains the lock
        tasks.push( task{ptr} );
        cv.notify_one();
    }

    template<class S>
    void submit_internal(std::function<S()>& ptr, return_value_handle<S> rv_handle) {
        std::unique_lock<std::mutex> lock(queue_stop_mutex);

        write_task(
            [ptr, rv_handle] () mutable {
                    rv_handle.set_value(ptr());
                }
            );
    }

public:
    threadpool(const int& threads);
    ~threadpool();

    template<class T>
    [[nodiscard]]
    return_value_handle<T> submit(const std::function<T()>& ptr) {
        std::unique_lock<std::mutex> lock(queue_stop_mutex);
        if (m_Stop) {
            lock.unlock();
            throw std::runtime_error{"ThreadPool::submit() after shutdown called"};
        }

        return_value_handle<T> rv_handle{};

        write_task(
            [ptr, rv_handle] () mutable {
                    rv_handle.set_value(ptr());
                }
            );
        return rv_handle;
    }

    template<class T>
    [[nodiscard]]
    return_value_handle<T> submit(const std::function<T()>& ptr, int dependency_id) {
        return_value_handle<T> rv_handle{};
        return rv_handle;
    }


    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    int queue_size();

    // Dependency DAG API
    template<typename... Args>
    return_value_handle<void> when_all(Args... args) {
        // Todo: actually implement the logic
        return_value_handle<void> rv{};
        return rv;
    }


};

// Void specialization
template<>
inline return_value_handle<void> threadpool::submit(const std::function<void()>& ptr) {

    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    if (m_Stop) {
        lock.unlock();
        throw std::runtime_error{"ThreadPool::submit() after shutdown called"};
    }

    return_value_handle<void> rv_handle{};

    write_task(
        [ptr] (){
                ptr();
            }
        );
    return rv_handle;
}

template<>
void threadpool::submit_internal(std::function<void()>& ptr, return_value_handle<void> rv_handle) {
    std::unique_lock<std::mutex> lock(queue_stop_mutex);

    write_task(
        [ptr, rv_handle] () mutable {
                ptr();
            }
        );
}
