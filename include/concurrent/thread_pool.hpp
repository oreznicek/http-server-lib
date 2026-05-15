#ifndef _CONCURRENT_THREAD_POOL_HPP
#define _CONCURRENT_THREAD_POOL_HPP

#include "concurrent/queue.hpp"

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

namespace concurrent {

/// @brief Generates a readable, sequential ID for the current thread.
inline int get_short_thread_id() {
    static std::atomic<int> global_id_counter{1};
    thread_local int my_short_id = global_id_counter++;
    return my_short_id;
}

/// @brief Represents a single unit of executable work.
using Task = std::move_only_function<void()>;

/**
 * @brief Thread Pool for concurrent task execution.
 *
 * @details Manages a fixed pool of worker threads that pull tasks
 *          from a thread-safe `Queue`.
 */
class ThreadPool {
    std::vector<std::jthread> workers_;
    Queue<Task> task_queue_;
public:
    /**
     * @brief Constructs the Thread Pool and spins up worker threads.
     *
     * @details Instantiates the specified number of worker threads.
     *          Each worker enters an infinite loop, blocking until a task
     *          is available in the queue.
     *
     * @param thread_count The number of worker threads to spawn.
     */
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

    /**
     * @brief Submits a new task to the thread pool for execution.
     *
     * @param task The callable work unit to execute.
     * @return true if the task was successfully pushed to the queue.
     * @return false if the queue is closed.
     */
    bool submit_task(Task&& task)
    {
        return task_queue_.push(std::move(task));
    }

    /**
     * @brief Gracefully shuts down the thread pool.
     *
     * @details Signals the underlying queue to close, preventing any
     *          new tasks from being submitted. Joins all threads, ensuring
     *          they finish before the program exits.
     */
    ~ThreadPool()
    {
        task_queue_.close();
    }
};

} // end of `concurrent` namespace

#endif
