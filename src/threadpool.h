#pragma once
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <thread>
#include <mutex>

// Wrapper for the function pointers
struct task {
    task(std::function<void()> ptr) : m_Ptr {ptr} {
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

    friend class threadpool; // Allow access to mutate m_Value and m_IsValid

    return_value() = default;

    return_value(const return_value&) = delete; // Copy Constructor
    return_value& operator=(const return_value&) = delete; // Copy Assignment Constructor

    return_value(return_value&&) = delete; // Move Constructor
    return_value& operator=(return_value&&) = delete; // Move Assignment Constructor

    bool is_valid() const {
        return m_IsValid;
    }

    const T& get() {
        std::lock_guard<std::mutex> lock(access_mutex);
        if (!is_valid()) {
            throw std::runtime_error("Thread Return Value is Invalid!");
        }
        return m_Value;
    }

private:
    std::mutex access_mutex;
    T m_Value;
    std::atomic<bool> m_IsValid{false};
};

template<class T>
struct return_value_handle {
public:
    return_value_handle() : m_Value{std::make_shared<return_value<T>>()} {
    }

    return_value_handle(const return_value_handle&) = default; // Copy Constructor
    return_value_handle& operator=(const return_value_handle&) = default; // Copy Assignment Constructor

    return_value_handle(return_value_handle&&) = default; // Move Constructor
    return_value_handle& operator=(return_value_handle&&) = default; // Move Assignment Constructor

    bool is_valid() const {
        if (m_Value == nullptr)
            return false;

        return m_Value -> is_valid();
    }

    const T& get() const {
        return m_Value.get()->get();
    }

private:
    std::shared_ptr<return_value<T>> m_Value;
};


// Specialization of void
template<>
struct return_value<void> {

    friend class threadpool; // Allow access to mutate m_Value and m_IsValid

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
        if (!is_valid()) throw std::runtime_error("Thread Return Value is Invalid!");
        // nothing to return
    }

private:
    std::mutex access_mutex;
    std::atomic<bool> m_IsValid{false};
};

template<>
struct return_value_handle<void> {
public:
    return_value_handle() : m_Value{std::make_shared<return_value<void>>()} {
    }

    return_value_handle(const return_value_handle&) = default; // Copy Constructor
    return_value_handle& operator=(const return_value_handle&) = default; // Copy Assignment Constructor

    return_value_handle(return_value_handle&&) = default; // Move Constructor
    return_value_handle& operator=(return_value_handle&&) = default; // Move Assignment Constructor

    bool is_valid() const {
        if (m_Value == nullptr)
            return false;

        return m_Value -> is_valid();
    }

    void get() const {
        return m_Value.get() -> get();
    }

private:
    std::shared_ptr<return_value<void>> m_Value;
};











class threadpool {
private:
    std::queue<task> tasks;
    std::vector<std::thread> workers;

    std::atomic<bool> m_IsRunning{true};
    std::mutex queue_mutex;

    std::optional<task> poll_task();
    bool write_task(std::function<void()> ptr) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        // Todo: custom logic to make sure that the queue can actually take the task
        tasks.push( task{ptr} );
        return true;
    }

public:
    threadpool(const int& threads);

    template<class T>
    return_value<T> submit(const std::function<T()>& ptr) {

        return_value<T> rv{};

        write_task(
            [ptr, rv] () mutable {
                    rv.m_Value = ptr();
                    rv.m_IsValid = true;
                }
            );
        return rv;
    }

    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    int queue_size();
};

