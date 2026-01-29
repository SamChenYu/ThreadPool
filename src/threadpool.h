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

    void write_task(const std::function<void()>& fn) {
        // No lock guard as submit() already contains the lock
        tasks.push(fn);
        cv.notify_one();
    }

public:
    explicit threadpool(const int& threads);
    ~threadpool();


    template<typename Function, typename... Args>

    auto submit(Function &&F, Args &&...ArgList) {

        using ReturnType = std::invoke_result_t<Function, Args...>;

        std::shared_ptr<std::packaged_task<ReturnType()>> task = std::make_shared<std::packaged_task<ReturnType()>>((
            std::bind(std::forward<Function>(F),
                      std::forward<Args>(ArgList)...)
        ));

        auto future = task->get_future();

        write_task([task]() mutable { (*task)(); });


        return future; // Return type is future<ReturnType>
    }

    // template<std::invocable Fn>
    // [[nodiscard]]
    // auto submit(const Fn&& fn) {
    //     using return_type = std::invoke_result_t<Fn>;
    //
    //     std::unique_lock lock(queue_stop_mutex);
    //     if (m_Stop) {
    //         throw std::runtime_error{"ThreadPool::submit() after shutdown called"};
    //     }
    //
    //     std::packaged_task<return_type> task{fn};
    //     write_task([&task]() { task(); });
    //     return task.get_future();
    // }


    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    size_t queue_size();

    // Dependency DAG API
    // template<typename... Args>
    // return_value_handle<void> when_all(Args... args) {
    //     // Todo: actually implement the logic
    //     return_value_handle<void> rv{};
    //     return rv;
    // }


};