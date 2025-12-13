#pragma once

#include "../cache/PageCache.h"
#include "../metrics/Counters.h"
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace pagecache
{

    class WritebackEngine
    {
    public:
        explicit WritebackEngine(std::shared_ptr<PageCache> cache);
        ~WritebackEngine();

        void start();
        void stop();
        void fsync(uint64_t file_id);
        void set_dirty_threshold(size_t threshold) { dirty_threshold_ = threshold; }

    private:
        std::shared_ptr<PageCache> cache_;
        std::atomic<bool> running_;
        std::thread writeback_thread_;
        std::mutex lock_;
        std::condition_variable cv_;
        size_t dirty_threshold_;
        std::shared_ptr<Counters> counters_;

        void writeback_loop();
        void flush_dirty_pages();
    };

}
