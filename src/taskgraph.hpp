#pragma once
#include "threadpool.hpp"
#include <queue>
#include <utility>
#include <vector>
#include <future>
#include <functional>

class taskgraph {
private:
    struct task_wrapper {
        explicit task_wrapper(threadpool& tp) : m_tp_ptr(&tp), m_dependency_count{0}, m_ready_dependencies{0} {
        }
        std::function<void()> m_task;
        std::vector<std::function<void()>> m_callbacks;
        threadpool* m_tp_ptr;
        
        const int m_dependency_count;
        int m_ready_dependencies;
        
        void set_task(const std::function<void()>& fn) {
            m_task = fn;
        }

        auto try_submit() {
            if (m_dependency_count == m_ready_dependencies) {
                return m_tp_ptr->submit(m_task);
            }
        }
    };

    template<class T>
    struct future_wrapper {
        explicit future_wrapper(std::future<T> fut, const task_wrapper* ptr) : m_fut{std::move(fut)}, m_ptr{ptr} {
        }
        std::future<T> m_fut;
        const task_wrapper* m_ptr;

        auto get() {
            return m_fut.get();
        }
    };

    threadpool* m_tp;
    std::vector<std::shared_ptr<task_wrapper>> task_registry{};
    bool m_start{false};

public:
    explicit taskgraph(const int& n) : m_tp(new threadpool(n)) {
    }

    inline ~taskgraph() {
        delete m_tp;
    }

    template<typename Function, typename... Args>
    [[nodiscard]]
    auto enqueue(Function &&F, Args  &&...ArgList) {

        // Todo:
        // do some TMP and iterate over the Args for a future_wrapper to register the callback


        std::shared_ptr<task_wrapper> task_wrapper_ptr = std::make_shared<task_wrapper>(*m_tp);

        using ReturnType = std::invoke_result_t<Function, Args...>;
        
        std::shared_ptr<std::packaged_task<ReturnType()>> sub_task = std::make_shared<std::packaged_task<ReturnType()>>((
            std::bind(std::forward<Function>(F),
                      std::forward<Args>(ArgList)...)
        ));

        // super task is actually ran by the threadpool worker
        // this has no future because it doesn't return anything 😭
        auto super_task = [sub_task, task_wrapper_ptr](){
            (*sub_task)();

            for(const auto& f : task_wrapper_ptr->m_callbacks) {
                f();
            }

            task_wrapper_ptr->m_callbacks.clear();
        };

        task_wrapper_ptr->set_task(super_task);
        auto future = sub_task->get_future(); // the actual future we want
        task_registry.push_back(task_wrapper_ptr);
        task_wrapper_ptr->try_submit();
        return future_wrapper<ReturnType>{std::move(future), task_wrapper_ptr.get()}; 
    }

    void start() {
        // topological sort
    }
};
