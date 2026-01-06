#include "threadpool.h"



// ============ THREADPOOL PUBLIC ============

threadpool::threadpool(const int& n) {
    workers.reserve(n);
    for (int i=0; i<n; i++) {
        workers[i] = std::thread([this, i]() {

            while (this->m_IsRunning) {

                std::optional<task> opt_task = this->poll_task();
                if (opt_task.has_value()) {
                    auto& task = opt_task.value();
                    task.m_Ptr();
                }
            }

        });
    }
}



void threadpool::shutdown() {
    m_IsRunning = false;
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void threadpool::shutdown_now() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    m_IsRunning = false;
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
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (tasks.empty()) {
        return std::nullopt;
    }
    task front = tasks.front();
    tasks.pop();
    return front;
}


