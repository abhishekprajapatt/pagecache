#pragma once

#include "../cache/PageCache.h"
#include <memory>
#include <cstdint>

namespace pagecache
{

    class Readahead
    {
    public:
        explicit Readahead(std::shared_ptr<PageCache> cache);
        ~Readahead();

        void on_sequential_read(uint64_t file_id, uint64_t page_index);
        void set_readahead_window(size_t pages) { window_size_ = pages; }

    private:
        std::shared_ptr<PageCache> cache_;
        uint64_t last_file_id_;
        uint64_t last_page_index_;
        size_t window_size_;

        void prefetch_pages(uint64_t file_id, uint64_t start_page, size_t count);
    };

}
