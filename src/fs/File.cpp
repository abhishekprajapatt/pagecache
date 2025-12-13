#include "File.h"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

namespace pagecache
{

    File::File(std::shared_ptr<Inode> inode, FileMode mode, std::shared_ptr<PageCache> cache)
        : inode_(inode), mode_(mode), offset_(0), cache_(cache)
    {
    }

    File::~File()
    {
    }

    size_t File::read(uint8_t *buffer, size_t count)
    {
        std::lock_guard<std::mutex> lock(file_lock_);

        if (mode_ == FileMode::WriteOnly)
        {
            return 0;
        }

        size_t bytes_read = 0;
        uint64_t remaining = count;
        uint64_t current_offset = offset_;

        while (remaining > 0 && current_offset < inode_->size())
        {
            uint64_t page_index = current_offset / Page::PAGE_SIZE;
            uint64_t page_offset = current_offset % Page::PAGE_SIZE;
            size_t to_read = std::min(remaining, Page::PAGE_SIZE - page_offset);
            to_read = std::min(to_read, (size_t)(inode_->size() - current_offset));

            auto loader = [this, page_index](uint8_t *page_data)
            {
                return read_from_disk(page_data, page_index * Page::PAGE_SIZE, Page::PAGE_SIZE) > 0;
            };

            auto page = cache_->get_or_load(inode_->ino(), page_index, loader);
            if (!page)
            {
                break;
            }

            std::memcpy(buffer + bytes_read, page->data() + page_offset, to_read);
            page->decrement_refcount();

            bytes_read += to_read;
            remaining -= to_read;
            current_offset += to_read;
        }

        offset_ = current_offset;
        return bytes_read;
    }

    size_t File::write(const uint8_t *buffer, size_t count)
    {
        std::lock_guard<std::mutex> lock(file_lock_);

        if (mode_ == FileMode::ReadOnly)
        {
            return 0;
        }

        size_t bytes_written = 0;
        uint64_t remaining = count;
        uint64_t current_offset = offset_;

        while (remaining > 0)
        {
            uint64_t page_index = current_offset / Page::PAGE_SIZE;
            uint64_t page_offset = current_offset % Page::PAGE_SIZE;
            size_t to_write = std::min(remaining, Page::PAGE_SIZE - page_offset);

            auto loader = [](uint8_t *)
            { return true; };
            auto page = cache_->get_or_load(inode_->ino(), page_index, loader);

            if (!page)
            {
                break;
            }

            std::memcpy(page->data() + page_offset, buffer + bytes_written, to_write);
            page->set_state(PageState::Dirty);
            page->decrement_refcount();

            uint64_t new_end = current_offset + to_write;
            if (new_end > inode_->size())
            {
                inode_->set_size(new_end);
            }

            bytes_written += to_write;
            remaining -= to_write;
            current_offset += to_write;
        }

        offset_ = current_offset;
        return bytes_written;
    }

    void File::sync()
    {
        std::lock_guard<std::mutex> lock(file_lock_);
    }

    size_t File::read_from_disk(uint8_t *buffer, uint64_t offset, size_t count)
    {
        if (inode_->file_descriptor() < 0)
        {
            return 0;
        }

        ssize_t result = pread(inode_->file_descriptor(), buffer, count, offset);
        return result > 0 ? result : 0;
    }

    size_t File::write_to_disk(const uint8_t *buffer, uint64_t offset, size_t count)
    {
        if (inode_->file_descriptor() < 0)
        {
            return 0;
        }

        ssize_t result = pwrite(inode_->file_descriptor(), buffer, count, offset);
        return result > 0 ? result : 0;
    }

}
