#include "../api/UserAPI.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace pagecache
{

    PageCacheSystem::PageCacheSystem()
        : next_ino_(1000)
    {
        cache_ = std::make_shared<PageCache>(65536);
        writeback_ = std::make_shared<WritebackEngine>(cache_);
        readahead_ = std::make_shared<Readahead>(cache_);
        counters_ = std::make_shared<Counters>();
        writeback_->start();
    }

    PageCacheSystem::~PageCacheSystem()
    {
        writeback_->stop();
    }

    PageCacheSystem &PageCacheSystem::instance()
    {
        static PageCacheSystem sys;
        return sys;
    }

    std::shared_ptr<File> PageCacheSystem::open_file(const std::string &path, FileMode mode)
    {
        auto inode = get_or_create_inode(path);

        if (inode->file_descriptor() < 0)
        {
            int flags = 0;
            if (mode == FileMode::ReadOnly)
            {
                flags = O_RDONLY;
            }
            else if (mode == FileMode::WriteOnly)
            {
                flags = O_WRONLY | O_CREAT;
            }
            else
            {
                flags = O_RDWR | O_CREAT;
            }

            int fd = open(path.c_str(), flags, 0644);
            if (fd >= 0)
            {
                inode->set_file_descriptor(fd);

                struct stat st;
                if (fstat(fd, &st) == 0)
                {
                    inode->set_size(st.st_size);
                }
            }
        }

        inode->increment_open_count();
        return std::make_shared<File>(inode, mode, cache_);
    }

    void PageCacheSystem::close_file(std::shared_ptr<File> file)
    {
        if (file && file->inode())
        {
            file->inode()->decrement_open_count();
        }
    }

    void PageCacheSystem::sync_all()
    {
        writeback_->fsync(0);
    }

    std::shared_ptr<Inode> PageCacheSystem::get_or_create_inode(const std::string &path)
    {
        std::lock_guard<std::mutex> lock(inode_lock_);

        uint64_t hash = std::hash<std::string>{}(path);

        if (inode_cache_.find(hash) != inode_cache_.end())
        {
            return inode_cache_[hash];
        }

        auto inode = std::make_shared<Inode>(next_ino_++, path);
        inode_cache_[hash] = inode;
        return inode;
    }

}
