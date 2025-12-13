#pragma once

#include "Inode.h"
#include "../cache/PageCache.h"
#include <memory>
#include <vector>
#include <mutex>

namespace pagecache
{

    enum class FileMode
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    class File
    {
    public:
        File(std::shared_ptr<Inode> inode, FileMode mode, std::shared_ptr<PageCache> cache);
        ~File();

        std::shared_ptr<Inode> inode() const { return inode_; }
        FileMode mode() const { return mode_; }
        uint64_t offset() const { return offset_; }
        void set_offset(uint64_t offset) { offset_ = offset; }

        size_t read(uint8_t *buffer, size_t count);
        size_t write(const uint8_t *buffer, size_t count);

        void seek(uint64_t offset) { offset_ = offset; }
        void sync();

    private:
        std::shared_ptr<Inode> inode_;
        FileMode mode_;
        uint64_t offset_;
        std::shared_ptr<PageCache> cache_;
        mutable std::mutex file_lock_;

        size_t read_from_disk(uint8_t *buffer, uint64_t offset, size_t count);
        size_t write_to_disk(const uint8_t *buffer, uint64_t offset, size_t count);
    };

}
