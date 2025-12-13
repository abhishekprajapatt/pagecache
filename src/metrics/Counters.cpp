#include "../metrics/Counters.h"

namespace pagecache
{

    Counters::Counters()
        : cache_hits_(0), cache_misses_(0), bytes_read_(0), bytes_written_(0),
          evictions_(0), writebacks_(0)
    {
    }

    Counters::~Counters()
    {
    }

}
