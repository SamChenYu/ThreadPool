#include "threadpool.h"

// ============ TASK ============

task::task(std::function<void()> ptr) {
    m_Ptr = ptr;
}

// ============ THREADPOOL PUBLIC ============

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




// ============ THREADPOOL PRIVATE ============
// Todo: USE AN RAII WRAPPER OF MUTEX OTHERWISE THERE COULD BE A DEADLOCK!!
std::optional<task> threadpool::poll_task() {
    read_tasks_mutex.lock();

    if (tasks.empty()) {
        return std::nullopt;
    }

    task front = tasks.front();
    tasks.pop();
    read_tasks_mutex.unlock();
    return front;
}

// Todo: Decide on whether to take a lambda or a std::function pointer
void threadpool::write_task() {

}

