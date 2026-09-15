#pragma once
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>


class threadpool {
private:
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> workers;

    bool m_Stop{false};
    std::mutex queue_stop_mutex; // Used for queue operations and read/write m_Stop operations
    std::condition_variable cv;

    inline void write_task(const std::function<void()>& fn) {
        // Does not need the lock as submit already acquires it
        tasks.push(fn);
        cv.notify_one();
    }


public:
    inline explicit threadpool(const int& n) {
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
    inline ~threadpool() {
        std::unique_lock<std::mutex> lock(queue_stop_mutex);
        if (!(m_Stop)) {
            lock.unlock(); // Unlock so that shutdown can use the mutex
            shutdown();
        }
    }

    template<typename Function, typename... Args>
    [[nodiscard]]
    auto submit(Function &&F, Args &&...ArgList) {

        std::unique_lock<std::mutex> lock(queue_stop_mutex);
        if (m_Stop) {
            throw std::runtime_error{"ThreadPool::submit() after shutdown called"};
        }

        using ReturnType = std::invoke_result_t<Function, Args...>;

        std::shared_ptr<std::packaged_task<ReturnType()>> task = std::make_shared<std::packaged_task<ReturnType()>>((
            std::bind(std::forward<Function>(F),
                      std::forward<Args>(ArgList)...)
        ));

        auto future = task->get_future();

        write_task([task](){ (*task)(); });

        return future;
    }

    // finish queued tasks
    inline void shutdown() {
        std::unique_lock<std::mutex> lock(queue_stop_mutex);
        m_Stop = true;
        cv.notify_all();
        lock.unlock();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    // cancel pending tasks
    void shutdown_now() {
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
    inline size_t queue_size() {
        std::lock_guard<std::mutex> lock(queue_stop_mutex);
        return tasks.size();
    }
};