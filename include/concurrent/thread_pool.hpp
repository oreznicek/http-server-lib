#ifndef _CONCURRENT_THREAD_POOL_HPP
#define _CONCURRENT_THREAD_POOL_HPP

#include "concurrent/queue.hpp"

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

namespace concurrent {

inline int get_short_thread_id() {
    static std::atomic<int> global_id_counter{1};
    thread_local int my_short_id = global_id_counter++;
    return my_short_id;
}

using Task = std::move_only_function<void()>;

class ThreadPool {
    std::vector<std::jthread> workers_;
    Queue<Task> task_queue_;
public:
    ThreadPool(std::size_t thread_count)
    {
        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this]() {
                Task task;
                while (task_queue_.pop(task)) {
                    task();
                    task = nullptr;
                }
            });
        }
    }

    void submit_task(Task&& task)
    {
        task_queue_.push(std::move(task));
    }

    ~ThreadPool()
    {
        task_queue_.close();
    }
};

} // end of `concurrent` namespace

#endif
