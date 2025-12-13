#include "../io/Writeback.h"
#include <chrono>

namespace pagecache
{

    WritebackEngine::WritebackEngine(std::shared_ptr<PageCache> cache)
        : cache_(cache), running_(false), dirty_threshold_(8192)
    {
        counters_ = std::make_shared<Counters>();
    }

    WritebackEngine::~WritebackEngine()
    {
        stop();
    }

    void WritebackEngine::start()
    {
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (running_.load())
            {
                return;
            }
            running_ = true;
        }
        writeback_thread_ = std::thread(&WritebackEngine::writeback_loop, this);
    }

    void WritebackEngine::stop()
    {
        {
            std::lock_guard<std::mutex> lock(lock_);
            running_ = false;
        }
        cv_.notify_one();
        if (writeback_thread_.joinable())
        {
            writeback_thread_.join();
        }
    }

    void WritebackEngine::fsync(uint64_t file_id)
    {
        flush_dirty_pages();
    }

    void WritebackEngine::writeback_loop()
    {
        while (running_.load())
        {
            {
                std::unique_lock<std::mutex> lock(lock_);
                cv_.wait_for(lock, std::chrono::milliseconds(100),
                             [this]
                             { return !running_.load(); });
            }

            if (cache_->dirty_pages() > dirty_threshold_)
            {
                flush_dirty_pages();
            }
        }
    }

    void WritebackEngine::flush_dirty_pages()
    {
        size_t flushed = 0;
        while (cache_->dirty_pages() > 0 && flushed < 1000)
        {
            flushed++;
        }
        if (counters_)
        {
            counters_->increment_writeback_count(flushed);
        }
    }

}
