#ifndef _CONCURRENT_QUEUE_HPP
#define _CONCURRENT_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

namespace concurrent {

/**
 * @brief A thread-safe queue designed for producer-consumer workflows.
 *
 * @tparam T The type of elements to be stored in the queue.
 */
template<typename T>
class Queue {
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
public:
    /**
     * @brief Attempts to safely push a new item onto the queue.
     *
     * @details Moves the item into the underlying container if the queue
     *          is still open. If successful, it wakes up exactly one sleeping
     *          consumer stuck in `pop()`.
     *
     * @param item The element to be pushed into the queue.
     * @return true if the item was successfully pushed.
     * @return false if the queue has been closed (the item is rejected).
     */
    bool push(T&& item)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (closed_) {
                return false;
            }
            queue_.push(std::move(item));
        }
        cv_.notify_one();
        return true;
    }

    /**
     * @brief Blocks the calling thread until an item is available or the queue is closed.
     *
     * @details If the queue is empty, the thread is put to sleep.
     *          If the queue is non-empty, threads will continue
     *          to wake up and pop items.
     *          Once the queue is both empty AND closed, this function
     *          returns false.
     *
     * @param item A reference to a variable where the popped item will be moved into.
     * @return true if an item was successfully extracted.
     * @return false if the queue is completely empty and securely closed.
     */
    bool pop(T& item)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]() {
            return !queue_.empty() || closed_;
        });

        if (queue_.empty() && closed_) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /// @brief Securely shuts down the queue and alerts all waiting threads.
    void close()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
    }
};

} // end of `concurrent` namespace

#endif
