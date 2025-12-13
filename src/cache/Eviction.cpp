#include "Eviction.h"
#include <algorithm>
#include <limits>

namespace pagecache
{

    std::shared_ptr<Page> LRUEviction::select_victim(
        const std::vector<std::shared_ptr<Page>> &candidates)
    {

        std::shared_ptr<Page> victim = nullptr;
        uint64_t oldest_access = std::numeric_limits<uint64_t>::max();

        for (const auto &page : candidates)
        {
            if (page->refcount() == 0 && !page->is_locked())
            {
                if (page->last_accessed() < oldest_access)
                {
                    oldest_access = page->last_accessed();
                    victim = page;
                }
            }
        }

        return victim;
    }

    std::shared_ptr<Page> CLOCKEviction::select_victim(
        const std::vector<std::shared_ptr<Page>> &candidates)
    {

        if (candidates.empty())
            return nullptr;

        size_t start_idx = clock_hand_ % candidates.size();
        size_t idx = start_idx;

        do
        {
            const auto &page = candidates[idx];
            if (page->refcount() == 0 && !page->is_locked())
            {
                clock_hand_ = (idx + 1) % candidates.size();
                return page;
            }
            idx = (idx + 1) % candidates.size();
        } while (idx != start_idx);

        return nullptr;
    }

}
