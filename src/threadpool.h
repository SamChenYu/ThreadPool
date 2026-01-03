#pragma once

#include <queue>
#include <vector>
#include <optional>

#include <functional>
#include <thread>
#include <future>
#include <mutex>

struct task {
    task(std::function<void()> ptr);
    std::function<void()> m_Ptr;
};

template<class T>
struct return_value {
    bool completed{false};
    T value;
};

class threadpool {
public:
    threadpool(const int& threads);

    template<class T>
    std::future<T> add_task(const std::function<void()>& task);
    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    int queue_size();

private:
    std::mutex write_tasks_mutex;
    std::mutex read_tasks_mutex;

    std::optional<task> poll_task();
    void write_task();

    std::queue<task> tasks;
    std::vector<std::thread> workers;
};
