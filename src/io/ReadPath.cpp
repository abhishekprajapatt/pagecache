#include "../io/IOPath.h"
#include <cstring>

namespace pagecache
{

    size_t ReadPath::execute(std::shared_ptr<PageCache> cache, uint64_t file_id,
                             uint64_t offset, uint8_t *buffer, size_t count)
    {
        size_t bytes_read = 0;
        uint64_t current_offset = offset;
        size_t remaining = count;

        while (remaining > 0)
        {
            uint64_t page_index = current_offset / Page::PAGE_SIZE;
            uint64_t page_offset = current_offset % Page::PAGE_SIZE;
            size_t to_read = std::min(remaining, Page::PAGE_SIZE - page_offset);

            auto loader = [](uint8_t *)
            { return true; };
            auto page = cache->get_or_load(file_id, page_index, loader);

            if (!page)
            {
                break;
            }

            std::memcpy(buffer + bytes_read, page->data() + page_offset, to_read);

            bytes_read += to_read;
            remaining -= to_read;
            current_offset += to_read;
        }

        return bytes_read;
    }

    size_t WritePath::execute(std::shared_ptr<PageCache> cache, uint64_t file_id,
                              uint64_t offset, const uint8_t *buffer, size_t count)
    {
        size_t bytes_written = 0;
        uint64_t current_offset = offset;
        size_t remaining = count;

        while (remaining > 0)
        {
            uint64_t page_index = current_offset / Page::PAGE_SIZE;
            uint64_t page_offset = current_offset % Page::PAGE_SIZE;
            size_t to_write = std::min(remaining, Page::PAGE_SIZE - page_offset);

            auto loader = [](uint8_t *)
            { return true; };
            auto page = cache->get_or_load(file_id, page_index, loader);

            if (!page)
            {
                break;
            }

            std::memcpy(page->data() + page_offset, buffer + bytes_written, to_write);
            page->set_state(PageState::Dirty);

            bytes_written += to_write;
            remaining -= to_write;
            current_offset += to_write;
        }

        return bytes_written;
    }

}
