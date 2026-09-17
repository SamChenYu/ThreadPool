#pragma once

#include "threadpool.hpp"
#include <queue>
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
 *
 *
 *
 *      so a bit more formally:
 *
 *      struct task_wrapper {
 *          
 *              
 *          std::package_task<ReturnType()> task;
 *          std::vector<std;:function()>> callbacks;
 *
 *          const int depency_count;
 *          int ready_depencies;
 *
 *          void ready_dependent();
 *
 *      }
 *
 *
 *
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
 *
 *
 *  because the approach is using an internal threadpool, there are some performance implications: the function pointers get double wrapped - first the taskgraph's layer firing callbacks, then after that the threadpool's internal wrapping. If there ever are performance concerns then it's possible to just rewrite the threadpool's internals and it it to the taskgraph
 */

class taskgraph {

};
