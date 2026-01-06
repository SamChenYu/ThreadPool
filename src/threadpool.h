#pragma once
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

template<class T>
struct return_value_handle;

// Wrapper for the function pointers
struct task {
    task(const std::function<void()>& ptr) : m_Ptr {ptr} {
    };

    void operator()() const {
        m_Ptr();
    }

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

    bool is_valid() const {
        return m_IsValid;
    }

    std::atomic<T>& get() {
        std::lock_guard<std::mutex> lock(access_mutex);
        if (!is_valid()) {
            throw std::runtime_error{"Thread Return Value is Invalid!"};
        }
        return m_Value;
    }

private:
    std::mutex access_mutex;
    std::atomic<T> m_Value;
    std::atomic<bool> m_IsValid{false};

    void set_value(const T& value) {
        m_Value = value;
    }
    void set_valid(const bool& value) {
        m_IsValid = value;
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

    bool is_valid() const {
        return m_IsValid;
    }

    void get() {
        std::lock_guard<std::mutex> lock(access_mutex);
        if (!is_valid()) throw std::runtime_error{"Thread Return Value is Invalid!"};
        // nothing to return
    }

private:
    std::mutex access_mutex;
    std::atomic<bool> m_IsValid{false};
    static void set_value() {
        // Do nothing
    }
    void set_valid(const bool& value) {
        m_IsValid = value;
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

    std::atomic<T>& get() const {
        return m_Handle.get()->get();
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

private:
    std::shared_ptr<return_value<void>> m_Handle;
    static void set_value() {
        // Do nothing
    }
};











class threadpool {
private:
    std::queue<task> tasks;
    std::vector<std::thread> workers;

    bool m_Stop{false};
    std::mutex queue_stop_mutex; // Used for queue operations and read/write m_Stop operations
    std::condition_variable cv;


    std::optional<task> poll_task();
    void write_task(const std::function<void()>& ptr) {
        std::lock_guard<std::mutex> lock(queue_stop_mutex);
        tasks.push( task{ptr} );
    }

public:
    threadpool(const int& threads);
    ~threadpool();

    template<class T>
    [[nodiscard]]
    return_value_handle<T> submit(const std::function<T()>& ptr) {
        return_value_handle<T> rv_handle{};

        write_task(
            [ptr, rv_handle] () mutable {
                    rv_handle.set_value(ptr());
                    rv_handle.set_valid(true);
                }
            );
        return rv_handle;
    }

    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    int queue_size();
};

// Void specialization
template<>
inline return_value_handle<void> threadpool::submit(const std::function<void()>& ptr) {

    return_value_handle<void> rv_handle{};

    write_task(
        [ptr] (){
                ptr();
            }
        );
    return rv_handle;
}
