cmake_minimum_required(VERSION 3.16)
project(pagecache VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE PAGECACHE_SOURCES "src/**/*.cpp")
list(REMOVE_ITEM PAGECACHE_SOURCES "${CMAKE_SOURCE_DIR}/src/bench/io_benchmark.cpp")

add_library(pagecache STATIC ${PAGECACHE_SOURCES})
target_link_libraries(pagecache PRIVATE pthread)

add_executable(benchmark src/bench/io_benchmark.cpp)
target_link_libraries(benchmark PRIVATE pagecache pthread)

enable_testing()

add_executable(test_page_cache tests/page_cache_tests.cpp)
target_link_libraries(test_page_cache PRIVATE pagecache pthread)
add_test(NAME PageCacheTests COMMAND test_page_cache)

add_executable(test_eviction tests/eviction_tests.cpp)
target_link_libraries(test_eviction PRIVATE pagecache pthread)
add_test(NAME EvictionTests COMMAND test_eviction)
