#ifndef _CONCURRENT_QUEUE_HPP
#define _CONCURRENT_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

namespace concurrent {

template<typename T>
class Queue {
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
public:
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
