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

    void write_task(const std::function<void()>& fn);

public:
    explicit threadpool(const int& threads);
    ~threadpool();

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


    void shutdown();   // finish queued tasks
    void shutdown_now(); // cancel pending tasks

    [[nodiscard]]
    size_t queue_size();
};