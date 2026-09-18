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
 *  t.start(); // topological sort
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
