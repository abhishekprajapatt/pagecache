#pragma once

#include <atomic>
#include <cstdint>
#include <chrono>

namespace pagecache
{

    class Counters
    {
    public:
        Counters();
        ~Counters();

        void increment_cache_hits() { cache_hits_.fetch_add(1, std::memory_order_relaxed); }
        void increment_cache_misses() { cache_misses_.fetch_add(1, std::memory_order_relaxed); }
        void increment_reads(size_t bytes) { bytes_read_.fetch_add(bytes, std::memory_order_relaxed); }
        void increment_writes(size_t bytes) { bytes_written_.fetch_add(bytes, std::memory_order_relaxed); }
        void increment_evictions() { evictions_.fetch_add(1, std::memory_order_relaxed); }
        void increment_writeback_count(size_t count) { writebacks_.fetch_add(count, std::memory_order_relaxed); }

        uint64_t cache_hits() const { return cache_hits_.load(std::memory_order_relaxed); }
        uint64_t cache_misses() const { return cache_misses_.load(std::memory_order_relaxed); }
        uint64_t bytes_read() const { return bytes_read_.load(std::memory_order_relaxed); }
        uint64_t bytes_written() const { return bytes_written_.load(std::memory_order_relaxed); }
        uint64_t evictions() const { return evictions_.load(std::memory_order_relaxed); }
        uint64_t writebacks() const { return writebacks_.load(std::memory_order_relaxed); }

        double hit_ratio() const
        {
            uint64_t hits = cache_hits_.load(std::memory_order_relaxed);
            uint64_t misses = cache_misses_.load(std::memory_order_relaxed);
            uint64_t total = hits + misses;
            return total > 0 ? (double)hits / total : 0.0;
        }

        void reset()
        {
            cache_hits_.store(0, std::memory_order_relaxed);
            cache_misses_.store(0, std::memory_order_relaxed);
            bytes_read_.store(0, std::memory_order_relaxed);
            bytes_written_.store(0, std::memory_order_relaxed);
            evictions_.store(0, std::memory_order_relaxed);
            writebacks_.store(0, std::memory_order_relaxed);
        }

    private:
        std::atomic<uint64_t> cache_hits_;
        std::atomic<uint64_t> cache_misses_;
        std::atomic<uint64_t> bytes_read_;
        std::atomic<uint64_t> bytes_written_;
        std::atomic<uint64_t> evictions_;
        std::atomic<uint64_t> writebacks_;
    };

}
