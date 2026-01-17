#include "threadpool.h"

// ============ THREADPOOL PUBLIC ============

threadpool::threadpool(const int& n) {
    workers.reserve(n);
    for (int i=0; i<n; i++) {
        workers.emplace_back([this]() {

            while (true) {
                std::unique_lock<std::mutex> lock(queue_stop_mutex);

                if (this->tasks.empty() && !(this->m_Stop)) {
                    this->cv.wait(lock, [this]() { return  ( !this->tasks.empty() || this->m_Stop ); });
                }

                if (this->tasks.empty() && this->m_Stop ) {
                    lock.unlock();
                    break;
                }
                std::optional<task> opt_task = this->poll_task();
                if (opt_task.has_value()) {
                    auto& task = opt_task.value();
                    lock.unlock();
                    task();
                }
            }

        });
    }
}

threadpool::~threadpool() {
    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    if (!(m_Stop)) {
        lock.unlock(); // Unlock so that shutdown can use the mutex
        shutdown();
    }
}

void threadpool::shutdown() {
    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    m_Stop = true;
    cv.notify_all();
    lock.unlock();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void threadpool::shutdown_now() {
    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    m_Stop = true;
    while (!tasks.empty()) {
        tasks.pop(); // Clear the queue
    }
    cv.notify_all();
    lock.unlock();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

[[nodiscard]]
int threadpool::queue_size() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    return tasks.size();
}

// ============ THREADPOOL PRIVATE ============

std::optional<task> threadpool::poll_task() {
    // No lock guard as the thread would already have the guard
    if (tasks.empty()) {
        return std::nullopt;
    }
    task front = tasks.front();
    tasks.pop();
    return front;
}