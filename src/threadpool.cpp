#include "threadpool.h"



// ============ THREADPOOL PUBLIC ============

threadpool::threadpool(const int& n) {
    workers.reserve(n);
    for (int i=0; i<n; i++) {
        workers[i] = std::thread([this]() {


            while (!(this->m_Stop)) {

                std::optional<task> opt_task = this->poll_task();
                if (opt_task.has_value()) {
                    auto& task = opt_task.value();
                    task.m_Ptr();
                }
            }




        });
    }
}

threadpool::~threadpool() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    if (!(m_Stop))
        shutdown();
}

void threadpool::shutdown() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    m_Stop = true;
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void threadpool::shutdown_now() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    m_Stop = true;
    while (!tasks.empty()) {
        tasks.pop(); // Clear the queue
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
}

[[nodiscard]]
int threadpool::queue_size() {
    return tasks.size();
}

// ============ THREADPOOL PRIVATE ============

std::optional<task> threadpool::poll_task() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    if (tasks.empty()) {
        return std::nullopt;
    }
    task front = tasks.front();
    tasks.pop();
    return front;
}


