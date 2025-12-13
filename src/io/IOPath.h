#pragma once

#include "../cache/Page.h"
#include "../cache/PageCache.h"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace pagecache
{

    class ReadPath
    {
    public:
        static size_t execute(std::shared_ptr<PageCache> cache, uint64_t file_id,
                              uint64_t offset, uint8_t *buffer, size_t count);
    };

    class WritePath
    {
    public:
        static size_t execute(std::shared_ptr<PageCache> cache, uint64_t file_id,
                              uint64_t offset, const uint8_t *buffer, size_t count);
    };

}
