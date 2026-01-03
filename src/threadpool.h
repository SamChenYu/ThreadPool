#pragma once

#include <queue>
#include <vector>

#include <functional>
#include <thread>
#include <future>

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
    std::queue<task> tasks;
    std::vector<std::thread> workers;
};
