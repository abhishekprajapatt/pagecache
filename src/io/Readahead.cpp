#include "../io/Readahead.h"

namespace pagecache
{

    Readahead::Readahead(std::shared_ptr<PageCache> cache)
        : cache_(cache), last_file_id_(0), last_page_index_(0), window_size_(8)
    {
    }

    Readahead::~Readahead()
    {
    }

    void Readahead::on_sequential_read(uint64_t file_id, uint64_t page_index)
    {
        if (file_id == last_file_id_ && page_index == last_page_index_ + 1)
        {
            prefetch_pages(file_id, page_index + 1, window_size_);
        }
        last_file_id_ = file_id;
        last_page_index_ = page_index;
    }

    void Readahead::prefetch_pages(uint64_t file_id, uint64_t start_page, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            uint64_t page_index = start_page + i;
            auto existing_page = cache_->get_page(file_id, page_index);
            if (!existing_page)
            {
                auto loader = [](uint8_t *)
                { return true; };
                cache_->get_or_load(file_id, page_index, loader);
            }
        }
    }

}
