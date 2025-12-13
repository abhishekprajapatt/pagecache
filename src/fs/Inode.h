#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace pagecache
{

    class Inode
    {
    public:
        explicit Inode(uint64_t ino, const std::string &path);
        ~Inode();

        uint64_t ino() const { return ino_; }
        const std::string &path() const { return path_; }

        uint64_t size() const { return size_; }
        void set_size(uint64_t size) { size_ = size; }

        int file_descriptor() const { return fd_; }
        void set_file_descriptor(int fd) { fd_ = fd; }

        uint64_t open_count() const { return open_count_; }
        void increment_open_count() { open_count_++; }
        void decrement_open_count()
        {
            if (open_count_ > 0)
                open_count_--;
        }

    private:
        uint64_t ino_;
        std::string path_;
        uint64_t size_;
        int fd_;
        uint64_t open_count_;
    };

}
