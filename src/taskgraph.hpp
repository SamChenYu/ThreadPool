#pragma once

#include "threadpool.hpp"
#include <queue>
#include <utility>
#include <vector>
#include <future>
#include <functional>

/*
 * public api:
 *  
 *  taskgraph t{2};
 *  future fut1 = taskgraph.enqueue(foo, 1, 2);
 *  future fut2 = taskgraph.enqueue_dependents(foo, fut1);
 *  
 *  auto val1 = fut1.get();
 *  auto val2 = fut2.get();
 *
 *  ok so here's the thing
 *      threadpool workers just take the task and invoke and thats all
 *      in the taskgraph, the task needs to also be wrapped to fire callbacks when the *      value is returned
 *
 *      these callbacks will fire to the dependent future_wrapper 
 *          these dependent future_wrappers will check if all the dependents are sati
 *          satisified, and if it is, then it will be enqueued to the internal threadpool

 *      so a bit more formally:
 *
 *      struct task_wrapper {
 *          std::package_task<ReturnType()> task;
 *          std::vector<std;:function()>> callbacks;
 *
 *          const int depency_count;
 *          int ready_depencies;
 *
 *          void ready_dependent();
 *
 *      }
 *      struct future_wrapper {
 *
 *          std::future val;
 *
 *          task_wrapper *ptr;
 *
 *      }
 *
 *      actual mechanics:
 *          - when you create a new task A
 *              - it's future_wrapper stores its parent task A
 *          - when you create a dependent task B and pass in future_wrapper A
 *              - it goes to the future_wrapper A's parent task, here is the dependent task, when you are done heres a callback to let me know when this particular future is done
 *
 *
 *          basicalliy:
 *             
 *           task A stores all dependent callbacks to fire later
 *           future A stores its parent task
 *
 *           task B / future B will store the same, but will be empty 
 *
 *          
 *
 *           so the lifetime of a dependnet is not stored inside an vector
 *           they will be heap allocated, and the upstream tasks will hold their pointers
 *           if they have all requirements satisifed, the task will enqeueu them onto the ready_task queue
 *           the lifetime keeps them alive

 *  because the approach is using an internal threadpool, there are some performance implications: the function pointers get double wrapped - first the taskgraph's layer firing callbacks, then after that the threadpool's internal wrapping. If there ever are performance concerns then it's possible to just rewrite the threadpool's internals and it it to the taskgraph
 */

class taskgraph {
private:
    struct task_wrapper {
        explicit task_wrapper(const std::function<void()>& f, std::queue<task_wrapper>& ready_tasks) : m_task(std::move(f)), m_ready_tasks{ready_tasks}, m_dependency_count{0}, m_ready_dependencies{0} {
        }
        std::function<void()> m_task;
        std::vector<std::function<void()>> m_callbacks;
        std::queue<task_wrapper>& m_ready_tasks;

        const int m_dependency_count;
        int m_ready_dependencies;

        void ready_dependent() const {
            if (m_ready_dependencies == m_dependency_count) {
                for (const auto& f : m_callbacks) {
                    f();
                }
            }
        };
    };

    template<class T>
    struct future_wrapper {
        explicit future_wrapper(const std::future<T>& fut, const task_wrapper* ptr) : m_fut{fut}, m_ptr{ptr} {

        }
        const std::future<T> m_fut;
        const task_wrapper *m_ptr;
    };

    threadpool m_tp;
    bool m_stop;

    inline auto write_task(const std::function<void()>& fn) {
        m_tp.submit // this needs to be submitted somehow
    }

public:
    explicit taskgraph(const int& n) : m_tp{n}, m_stop{false} {}

    template<typename Function, typename... Args>
    [[nodiscard]]
    auto submit(Function &&F, Args  &&...ArgList) {
        // will need to add mutexes to this in the future
        if (m_stop) {
            throw std::runtime_error{"taskgraph::submit() after shutdown called"};
        }

        task_wrapper task{}; // wrap the function, then how do you submit it?

        auto future = m_tp.submit(std::forward<Function>(F), std::forward<Args>(ArgList)...);
        return future_wrapper{std::move(future), this};
    }

};
