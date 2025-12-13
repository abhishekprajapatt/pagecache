#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <memory>

namespace pagecache {

enum class PageState {
    Clean,
    Dirty,
    Locked
};

class Page {
public:
    static constexpr size_t PAGE_SIZE = 4096;

    Page(uint64_t page_index);
    ~Page();

    uint64_t index() const { return index_; }
    uint8_t* data() { return data_.get(); }
    const uint8_t* data() const { return data_.get(); }
    
    PageState state() const { return state_; }
    void set_state(PageState s) { state_ = s; }
    
    uint32_t refcount() const { return refcount_.load(std::memory_order_acquire); }
    void increment_refcount() { refcount_.fetch_add(1, std::memory_order_acq_rel); }
    void decrement_refcount() { refcount_.fetch_sub(1, std::memory_order_acq_rel); }
    
    uint64_t last_accessed() const { return last_accessed_; }
    void touch() { last_accessed_ = next_timestamp(); }
    
    static uint64_t next_timestamp() { 
        static std::atomic<uint64_t> ts(0);
        return ts.fetch_add(1, std::memory_order_relaxed);
    }

    void lock() { locked_ = true; }
    void unlock() { locked_ = false; }
    bool is_locked() const { return locked_; }

private:
    uint64_t index_;
    std::unique_ptr<uint8_t[]> data_;
    PageState state_;
    std::atomic<uint32_t> refcount_;
    uint64_t last_accessed_;
    bool locked_;
};

}
