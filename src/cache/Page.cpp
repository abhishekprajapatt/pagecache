#include "Page.h"

namespace pagecache
{

    Page::Page(uint64_t page_index)
        : index_(page_index),
          data_(new uint8_t[PAGE_SIZE]),
          state_(PageState::Clean),
          refcount_(0),
          last_accessed_(next_timestamp()),
          locked_(false)
    {
    }

    Page::~Page()
    {
    }

}
