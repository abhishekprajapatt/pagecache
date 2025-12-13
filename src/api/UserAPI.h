#pragma once

#include "../cache/PageCache.h"
#include "../fs/File.h"
#include "../fs/Inode.h"
#include "../io/Writeback.h"
#include "../io/Readahead.h"
#include "../metrics/Counters.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>

namespace pagecache
{

    class PageCacheSystem
    {
    public:
        static PageCacheSystem &instance();

        std::shared_ptr<File> open_file(const std::string &path, FileMode mode);
        void close_file(std::shared_ptr<File> file);

        std::shared_ptr<PageCache> get_cache() { return cache_; }
        std::shared_ptr<Counters> get_counters() { return counters_; }

        void set_cache_size(size_t max_pages)
        {
            cache_ = std::make_shared<PageCache>(max_pages);
        }

        void set_eviction_policy(const std::string &policy)
        {
            cache_->set_eviction_policy(policy);
        }

        void sync_all();

    private:
        PageCacheSystem();
        ~PageCacheSystem();

        std::shared_ptr<PageCache> cache_;
        std::shared_ptr<WritebackEngine> writeback_;
        std::shared_ptr<Readahead> readahead_;
        std::shared_ptr<Counters> counters_;

        std::unordered_map<uint64_t, std::shared_ptr<Inode>> inode_cache_;
        std::mutex inode_lock_;
        uint64_t next_ino_;

        std::shared_ptr<Inode> get_or_create_inode(const std::string &path);
    };

}
