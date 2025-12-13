#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "cache/Page.h"
#include "cache/Eviction.h"

using namespace pagecache;

void test_lru_eviction_basic()
{
    std::vector<std::shared_ptr<Page>> pages;

    for (int i = 0; i < 5; ++i)
    {
        pages.push_back(std::make_shared<Page>(i));
    }

    LRUEviction lru;
    auto victim = lru.select_victim(pages);

    assert(victim != nullptr);
    std::cout << "✓ LRU eviction basic test passed" << std::endl;
}

void test_lru_respects_refcount()
{
    std::vector<std::shared_ptr<Page>> pages;

    auto p1 = std::make_shared<Page>(1);
    auto p2 = std::make_shared<Page>(2);

    p1->increment_refcount();

    pages.push_back(p1);
    pages.push_back(p2);

    LRUEviction lru;
    auto victim = lru.select_victim(pages);

    assert(victim == p2);
    std::cout << "✓ LRU respects refcount test passed" << std::endl;
}

void test_lru_respects_lock()
{
    std::vector<std::shared_ptr<Page>> pages;

    auto p1 = std::make_shared<Page>(1);
    auto p2 = std::make_shared<Page>(2);

    p1->lock();

    pages.push_back(p1);
    pages.push_back(p2);

    LRUEviction lru;
    auto victim = lru.select_victim(pages);

    assert(victim == p2);
    std::cout << "✓ LRU respects lock test passed" << std::endl;
}

void test_clock_eviction_basic()
{
    std::vector<std::shared_ptr<Page>> pages;

    for (int i = 0; i < 5; ++i)
    {
        pages.push_back(std::make_shared<Page>(i));
    }

    CLOCKEviction clock;
    auto victim = clock.select_victim(pages);

    assert(victim != nullptr);
    std::cout << "✓ CLOCK eviction basic test passed" << std::endl;
}

void test_clock_respects_refcount()
{
    std::vector<std::shared_ptr<Page>> pages;

    auto p1 = std::make_shared<Page>(1);
    auto p2 = std::make_shared<Page>(2);

    p1->increment_refcount();

    pages.push_back(p1);
    pages.push_back(p2);

    CLOCKEviction clock;
    auto victim = clock.select_victim(pages);

    assert(victim == p2);
    std::cout << "✓ CLOCK respects refcount test passed" << std::endl;
}

void test_clock_respects_lock()
{
    std::vector<std::shared_ptr<Page>> pages;

    auto p1 = std::make_shared<Page>(1);
    auto p2 = std::make_shared<Page>(2);

    p1->lock();

    pages.push_back(p1);
    pages.push_back(p2);

    CLOCKEviction clock;
    auto victim = clock.select_victim(pages);

    assert(victim == p2);
    std::cout << "✓ CLOCK respects lock test passed" << std::endl;
}

void test_eviction_with_all_locked()
{
    std::vector<std::shared_ptr<Page>> pages;

    auto p1 = std::make_shared<Page>(1);
    auto p2 = std::make_shared<Page>(2);

    p1->lock();
    p2->lock();

    pages.push_back(p1);
    pages.push_back(p2);

    LRUEviction lru;
    auto victim = lru.select_victim(pages);

    assert(victim == nullptr);
    std::cout << "✓ Eviction with all locked test passed" << std::endl;
}

int main()
{
    std::cout << "Running Eviction Strategy Tests\n"
              << std::endl;

    test_lru_eviction_basic();
    test_lru_respects_refcount();
    test_lru_respects_lock();
    test_clock_eviction_basic();
    test_clock_respects_refcount();
    test_clock_respects_lock();
    test_eviction_with_all_locked();

    std::cout << "\n✓ All eviction tests passed!" << std::endl;
    return 0;
}
