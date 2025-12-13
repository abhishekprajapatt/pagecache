#pragma once

#include "Page.h"
#include <unordered_map>
#include <memory>
#include <pthread.h>
#include <deque>
#include <vector>
#include <functional>

namespace pagecache
{

    class PageCache
    {
    public:
        explicit PageCache(size_t max_pages = 65536);
        ~PageCache();

        std::shared_ptr<Page> get_or_load(uint64_t file_id, uint64_t page_index,
                                          std::function<bool(uint8_t *)> loader);
        std::shared_ptr<Page> get_page(uint64_t file_id, uint64_t page_index);
        void insert_page(uint64_t file_id, uint64_t page_index, std::shared_ptr<Page> page);

        bool evict_one();
        void evict_to_target(size_t target_pages);

        size_t total_pages() const;
        size_t dirty_pages() const;
        size_t clean_pages() const;

        void set_eviction_policy(const std::string &policy);

    private:
        struct CacheEntry
        {
            std::shared_ptr<Page> page;
            uint64_t file_id;
        };

        size_t max_pages_;
        std::unordered_map<uint64_t, std::unordered_map<uint64_t, CacheEntry>> pages_by_file_;
        std::deque<std::pair<uint64_t, uint64_t>> lru_queue_;
        mutable pthread_mutex_t cache_lock_;
        std::string eviction_policy_;
        std::shared_ptr<Page> evict_lru();
        std::shared_ptr<Page> evict_clock();
        uint64_t make_key(uint64_t file_id, uint64_t page_index) const;
        void update_lru(uint64_t file_id, uint64_t page_index);
    };

}
