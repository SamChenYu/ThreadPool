#include "threadpool.h"

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

                if (!tasks.empty()) {
                    auto task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            }

        });
    }
}


void threadpool::write_task(const std::function<void()>& fn) {
    // Does not need the lock as submit already acquires it
    tasks.push(fn);
    cv.notify_one();
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
size_t threadpool::queue_size() {
    std::lock_guard<std::mutex> lock(queue_stop_mutex);
    return tasks.size();
}
