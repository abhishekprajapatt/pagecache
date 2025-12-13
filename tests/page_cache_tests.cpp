#include <iostream>
#include <cassert>
#include <cstring>
#include "cache/Page.h"
#include "cache/PageCache.h"

using namespace pagecache;

void test_page_creation()
{
    Page page(0);
    assert(page.index() == 0);
    assert(page.state() == PageState::Clean);
    assert(page.refcount() == 0);
    assert(!page.is_locked());
    std::cout << "✓ Page creation test passed" << std::endl;
}

void test_page_state_transitions()
{
    Page page(1);

    page.set_state(PageState::Dirty);
    assert(page.state() == PageState::Dirty);

    page.set_state(PageState::Clean);
    assert(page.state() == PageState::Clean);

    std::cout << "✓ Page state transition test passed" << std::endl;
}

void test_refcounting()
{
    Page page(2);

    page.increment_refcount();
    assert(page.refcount() == 1);

    page.increment_refcount();
    assert(page.refcount() == 2);

    page.decrement_refcount();
    assert(page.refcount() == 1);

    std::cout << "✓ Page refcounting test passed" << std::endl;
}

void test_page_locking()
{
    Page page(3);

    assert(!page.is_locked());
    page.lock();
    assert(page.is_locked());
    page.unlock();
    assert(!page.is_locked());

    std::cout << "✓ Page locking test passed" << std::endl;
}

void test_page_data()
{
    Page page(4);
    uint8_t test_data[Page::PAGE_SIZE];
    std::memset(test_data, 'a', sizeof(test_data));

    std::memcpy(page.data(), test_data, sizeof(test_data));
    assert(std::memcmp(page.data(), test_data, sizeof(test_data)) == 0);

    std::cout << "✓ Page data test passed" << std::endl;
}

void test_page_cache_basic()
{
    PageCache cache(100);

    auto loader = [](uint8_t *data)
    {
        std::memset(data, 'b', Page::PAGE_SIZE);
        return true;
    };

    auto page = cache.get_or_load(1, 0, loader);
    assert(page != nullptr);
    assert(page->state() == PageState::Clean);

    std::cout << "✓ PageCache basic test passed" << std::endl;
}

void test_cache_hit()
{
    PageCache cache(100);

    auto loader = [](uint8_t *data)
    {
        std::memset(data, 'c', Page::PAGE_SIZE);
        return true;
    };

    auto page1 = cache.get_or_load(2, 0, loader);
    auto page2 = cache.get_page(2, 0);

    assert(page1 == page2);

    std::cout << "✓ Cache hit test passed" << std::endl;
}

void test_cache_miss()
{
    PageCache cache(100);

    auto page = cache.get_page(3, 0);
    assert(page == nullptr);

    std::cout << "✓ Cache miss test passed" << std::endl;
}

void test_eviction()
{
    PageCache cache(3);

    auto loader = [](uint8_t *data)
    {
        std::memset(data, 'd', Page::PAGE_SIZE);
        return true;
    };

    cache.get_or_load(4, 0, loader);
    cache.get_or_load(4, 1, loader);
    cache.get_or_load(4, 2, loader);

    assert(cache.total_pages() == 3);

    cache.get_or_load(4, 3, loader);
    assert(cache.total_pages() == 3);

    std::cout << "✓ Eviction test passed" << std::endl;
}

void test_dirty_tracking()
{
    PageCache cache(100);

    auto loader = [](uint8_t *data)
    {
        std::memset(data, 'e', Page::PAGE_SIZE);
        return true;
    };

    auto page = cache.get_or_load(5, 0, loader);
    assert(page->state() == PageState::Clean);
    assert(cache.clean_pages() == 1);
    assert(cache.dirty_pages() == 0);

    page->set_state(PageState::Dirty);
    assert(cache.dirty_pages() == 1);

    std::cout << "✓ Dirty tracking test passed" << std::endl;
}

int main()
{
    std::cout << "Running PageCache Tests\n"
              << std::endl;

    test_page_creation();
    test_page_state_transitions();
    test_refcounting();
    test_page_locking();
    test_page_data();
    test_page_cache_basic();
    test_cache_hit();
    test_cache_miss();
    test_eviction();
    test_dirty_tracking();

    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}
