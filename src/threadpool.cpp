#include "threadpool.h"



task::task(std::function<void()> ptr) {
    m_Ptr = ptr;
}

threadpool::threadpool(const int& n) {
    workers = std::vector<std::thread>(n);
    for (int i=0; i<n; i++) {
        workers[i] = std::thread([this]() {
            // Todo: Main worker logic
        });
    }
}

template<class T>
std::future<T> threadpool::add_task(const std::function<void()>& task) {
    // Todo:  Wrap this properly, return the future!
    tasks.emplace(task);
}

void threadpool::shutdown() {
    for (std::thread& worker : workers) {
        worker.join();
    }
}


[[nodiscard]]
int threadpool::queue_size() {
    return tasks.size();
}