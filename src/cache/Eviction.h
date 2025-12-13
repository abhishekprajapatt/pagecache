#pragma once

#include "Page.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace pagecache
{

    enum class EvictionPolicy
    {
        LRU,
        CLOCK
    };

    class EvictionStrategy
    {
    public:
        virtual ~EvictionStrategy() = default;
        virtual std::shared_ptr<Page> select_victim(
            const std::vector<std::shared_ptr<Page>> &candidates) = 0;
    };

    class LRUEviction : public EvictionStrategy
    {
    public:
        std::shared_ptr<Page> select_victim(
            const std::vector<std::shared_ptr<Page>> &candidates) override;
    };

    class CLOCKEviction : public EvictionStrategy
    {
    public:
        CLOCKEviction() : clock_hand_(0) {}
        std::shared_ptr<Page> select_victim(
            const std::vector<std::shared_ptr<Page>> &candidates) override;

    private:
        size_t clock_hand_;
    };

}
