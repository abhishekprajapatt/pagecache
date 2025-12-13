#include "../scheduler/IOThreadPool.h"

namespace pagecache
{

    IOThreadPool::IOThreadPool(size_t num_threads)
        : num_threads_(num_threads), shutdown_flag_(false), pending_tasks_(0)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            threads_.emplace_back(&IOThreadPool::worker_loop, this);
        }
    }

    IOThreadPool::~IOThreadPool()
    {
        shutdown();
    }

    void IOThreadPool::submit(Task task)
    {
        {
            std::lock_guard<std::mutex> lock(queue_lock_);
            task_queue_.push(task);
            pending_tasks_.fetch_add(1, std::memory_order_release);
        }
        cv_.notify_one();
    }

    void IOThreadPool::wait_all()
    {
        while (pending_tasks_.load(std::memory_order_acquire) > 0)
        {
            std::this_thread::yield();
        }
    }

    void IOThreadPool::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(queue_lock_);
            shutdown_flag_ = true;
        }
        cv_.notify_all();

        for (auto &thread : threads_)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void IOThreadPool::worker_loop()
    {
        while (true)
        {
            Task task = nullptr;
            {
                std::unique_lock<std::mutex> lock(queue_lock_);
                cv_.wait(lock, [this]
                         { return !task_queue_.empty() || shutdown_flag_.load(); });

                if (shutdown_flag_.load() && task_queue_.empty())
                {
                    break;
                }

                if (!task_queue_.empty())
                {
                    task = task_queue_.front();
                    task_queue_.pop();
                }
            }

            if (task)
            {
                task();
                pending_tasks_.fetch_sub(1, std::memory_order_release);
            }
        }
    }

}
