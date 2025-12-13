#pragma once

#include <memory>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace pagecache
{

    using Task = std::function<void()>;

    class IOThreadPool
    {
    public:
        explicit IOThreadPool(size_t num_threads = 4);
        ~IOThreadPool();

        void submit(Task task);
        void wait_all();
        void shutdown();

    private:
        size_t num_threads_;
        std::vector<std::thread> threads_;
        std::queue<Task> task_queue_;
        std::mutex queue_lock_;
        std::condition_variable cv_;
        std::atomic<bool> shutdown_flag_;
        std::atomic<size_t> pending_tasks_;

        void worker_loop();
    };

}
