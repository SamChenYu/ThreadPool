#include "threadpool.h"

#include <iostream>


// ============ THREADPOOL PUBLIC ============

threadpool::threadpool(const int& n) {
    workers.reserve(n);
    for (int i=0; i<n; i++) {
        workers[i] = std::thread([this, i]() {

            while (true) {
                std::unique_lock<std::mutex> lock(queue_stop_mutex);

                if (this->tasks.empty() && !(this->m_Stop)) {
                    std::cout << i << " Sleeping " << this->tasks.size() << std::endl;

                    this->cv.wait(lock, [this]() { return  ( !this->tasks.empty() || this->m_Stop ); });

                }

                std::cout << i << " Woken up " << this->tasks.size() << std::endl;
                if (this->tasks.empty() && (this->m_Stop) ) {

                    std::cout << i <<" Exiting " << this->tasks.size()  << std::endl;
                    lock.unlock();
                    break;
                }
                std::cout << i <<" Checking " << this->tasks.size() << std::endl;
                std::optional<task> opt_task = this->poll_task();
                if (opt_task.has_value()) {
                    std::cout << i <<" Doing work " << this->tasks.size()  << std::endl;
                    auto& task = opt_task.value();
                    lock.unlock();
                    task.m_Ptr();
                }
                std::cout << i << " Looping " << this->tasks.size() << std::endl;
            }

            // while (!(this->m_Stop)) {
            //
            //     std::optional<task> opt_task = this->poll_task();
            //     if (opt_task.has_value()) {
            //         auto& task = opt_task.value();
            //         task.m_Ptr();
            //     }
            // }




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
    std::cout << "Shutdown called " <<  std::endl;
    m_Stop = true;
    lock.unlock();
    cv.notify_all();
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
    // No lock guard as the thread would already have the guard
    if (tasks.empty()) {
        return std::nullopt;
    }
    task front = tasks.front();
    tasks.pop();
    return front;
}


