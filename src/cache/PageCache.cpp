#include "PageCache.h"
#include <algorithm>

namespace pagecache
{

    PageCache::PageCache(size_t max_pages)
        : max_pages_(max_pages), eviction_policy_("lru")
    {
    }

    PageCache::~PageCache()
    {
    }

    std::shared_ptr<Page> PageCache::get_or_load(uint64_t file_id, uint64_t page_index,
                                                 std::function<bool(uint8_t *)> loader)
    {
        pthread_mutex_lock(&cache_lock_);

        if (pages_by_file_[file_id].find(page_index) != pages_by_file_[file_id].end())
        {
            auto &entry = pages_by_file_[file_id][page_index];
            entry.page->touch();
            update_lru(file_id, page_index);
            return entry.page;
        }

        while (total_pages() >= max_pages_)
        {
            evict_one();
        }

        auto new_page = std::make_shared<Page>(page_index);
        new_page->lock();

        lock.unlock();
        bool success = loader(new_page->data());
        lock.lock();

        if (!success)
        {
            return nullptr;
        }

        new_page->unlock();
        new_page->set_state(PageState::Clean);
        new_page->touch();

        pages_by_file_[file_id][page_index] = {new_page, file_id};
        lru_queue_.push_back({file_id, page_index});

        return new_page;
    }

    std::shared_ptr<Page> PageCache::get_page(uint64_t file_id, uint64_t page_index)
    {
        std::lock_guard<std::mutex> lock(cache_lock_);

        if (pages_by_file_.find(file_id) == pages_by_file_.end())
        {
            return nullptr;
        }

        auto &file_cache = pages_by_file_[file_id];
        if (file_cache.find(page_index) == file_cache.end())
        {
            return nullptr;
        }

        return file_cache[page_index].page;
    }

    void PageCache::insert_page(uint64_t file_id, uint64_t page_index, std::shared_ptr<Page> page)
    {
        std::lock_guard<std::mutex> lock(cache_lock_);

        pages_by_file_[file_id][page_index] = {page, file_id};
        lru_queue_.push_back({file_id, page_index});
    }

    size_t PageCache::total_pages() const
    {
        std::lock_guard<std::mutex> lock(cache_lock_);
        size_t count = 0;
        for (const auto &file_entry : pages_by_file_)
        {
            count += file_entry.second.size();
        }
        return count;
    }

    size_t PageCache::dirty_pages() const
    {
        std::lock_guard<std::mutex> lock(cache_lock_);
        size_t count = 0;
        for (const auto &file_entry : pages_by_file_)
        {
            for (const auto &page_entry : file_entry.second)
            {
                if (page_entry.second.page->state() == PageState::Dirty)
                {
                    count++;
                }
            }
        }
        return count;
    }

    size_t PageCache::clean_pages() const
    {
        return total_pages() - dirty_pages();
    }

    void PageCache::set_eviction_policy(const std::string &policy)
    {
        std::lock_guard<std::mutex> lock(cache_lock_);
        eviction_policy_ = policy;
    }

    bool PageCache::evict_one()
    {
        std::shared_ptr<Page> victim = nullptr;

        if (eviction_policy_ == "clock")
        {
            victim = evict_clock();
        }
        else
        {
            victim = evict_lru();
        }

        return victim != nullptr;
    }

    void PageCache::evict_to_target(size_t target_pages)
    {
        std::lock_guard<std::mutex> lock(cache_lock_);

        while (total_pages() > target_pages)
        {
            evict_one();
        }
    }

    std::shared_ptr<Page> PageCache::evict_lru()
    {
        for (auto it = lru_queue_.begin(); it != lru_queue_.end(); ++it)
        {
            uint64_t file_id = it->first;
            uint64_t page_index = it->second;

            auto &file_cache = pages_by_file_[file_id];
            auto page_it = file_cache.find(page_index);

            if (page_it == file_cache.end())
            {
                lru_queue_.erase(it);
                continue;
            }

            auto page = page_it->second.page;

            if (page->refcount() > 0 || page->is_locked())
            {
                continue;
            }

            if (page->state() == PageState::Dirty)
            {
                page->set_state(PageState::Clean);
            }

            file_cache.erase(page_it);
            lru_queue_.erase(it);
            return page;
        }

        return nullptr;
    }

    std::shared_ptr<Page> PageCache::evict_clock()
    {
        for (auto it = lru_queue_.begin(); it != lru_queue_.end(); ++it)
        {
            uint64_t file_id = it->first;
            uint64_t page_index = it->second;

            auto &file_cache = pages_by_file_[file_id];
            auto page_it = file_cache.find(page_index);

            if (page_it == file_cache.end())
            {
                lru_queue_.erase(it);
                continue;
            }

            auto page = page_it->second.page;

            if (page->refcount() > 0 || page->is_locked())
            {
                continue;
            }

            bool accessed = page->last_accessed() > (Page::next_timestamp() - 1000);

            if (accessed)
            {
                page->touch();
                continue;
            }

            if (page->state() == PageState::Dirty)
            {
                page->set_state(PageState::Clean);
            }

            file_cache.erase(page_it);
            lru_queue_.erase(it);
            return page;
        }

        return nullptr;
    }

    uint64_t PageCache::make_key(uint64_t file_id, uint64_t page_index) const
    {
        return (file_id << 32) | page_index;
    }

    void PageCache::update_lru(uint64_t file_id, uint64_t page_index)
    {
        auto it = std::find_if(lru_queue_.begin(), lru_queue_.end(),
                               [file_id, page_index](const auto &p)
                               {
                                   return p.first == file_id && p.second == page_index;
                               });
        if (it != lru_queue_.end())
        {
            lru_queue_.erase(it);
        }
        lru_queue_.push_back({file_id, page_index});
    }

}
