#include "threadpool.h"



// ============ THREADPOOL PUBLIC ============

threadpool::threadpool(const int& n) {
    workers.reserve(n);
    for (int i=0; i<n; i++) {
        workers[i] = std::thread([this]() {

            // while (true) {
            //     std::unique_lock<std::mutex> lock(queue_stop_mutex);
            //
            //     this->cv.wait(lock, [this]() { return this->m_Stop;});
            //
            //     if (this->tasks.empty() && !(this->m_Stop) ) {
            //         lock.unlock();
            //         break;
            //     }
            //     std::optional<task> opt_task = this->poll_task();
            //     if (opt_task.has_value()) {
            //         auto& task = opt_task.value();
            //         lock.unlock();
            //         task.m_Ptr();
            //     }
            // }

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
    std::unique_lock<std::mutex> lock(queue_stop_mutex);
    if (!(m_Stop)) {
        lock.unlock(); // Unlock so that shutdown can use the mutex
        shutdown();
    }
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
int threadpool::queue_size() const {
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


