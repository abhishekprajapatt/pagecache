#!/bin/bash
g++ -std=c++11 -Wall -Wextra -O3 -I. \
  src/cache/Page.cpp \
  src/cache/PageCache.cpp \
  src/cache/Eviction.cpp \
  src/fs/Inode.cpp \
  src/fs/File.cpp \
  src/io/ReadPath.cpp \
  src/io/Writeback.cpp \
  src/io/Readahead.cpp \
  src/scheduler/IOThreadPool.cpp \
  src/metrics/Counters.cpp \
  src/api/UserAPI.cpp \
  -pthread -o build/pagecache_lib

g++ -std=c++11 -Wall -Wextra -O3 -I. \
  src/cache/Page.cpp \
  src/cache/PageCache.cpp \
  src/cache/Eviction.cpp \
  src/fs/Inode.cpp \
  src/fs/File.cpp \
  src/io/ReadPath.cpp \
  src/io/Writeback.cpp \
  src/io/Readahead.cpp \
  src/scheduler/IOThreadPool.cpp \
  src/metrics/Counters.cpp \
  src/api/UserAPI.cpp \
  tests/page_cache_tests.cpp \
  -pthread -o build/test_page_cache

g++ -std=c++11 -Wall -Wextra -O3 -I. \
  src/cache/Page.cpp \
  src/cache/PageCache.cpp \
  src/cache/Eviction.cpp \
  src/fs/Inode.cpp \
  src/fs/File.cpp \
  src/io/ReadPath.cpp \
  src/io/Writeback.cpp \
  src/io/Readahead.cpp \
  src/scheduler/IOThreadPool.cpp \
  src/metrics/Counters.cpp \
  src/api/UserAPI.cpp \
  tests/eviction_tests.cpp \
  -pthread -o build/test_eviction

echo "Build complete"
