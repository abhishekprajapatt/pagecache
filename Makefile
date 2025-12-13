CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -pthread -I.

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

CACHE_SRCS = $(SRC_DIR)/cache/Page.cpp $(SRC_DIR)/cache/PageCache.cpp $(SRC_DIR)/cache/Eviction.cpp
FS_SRCS = $(SRC_DIR)/fs/Inode.cpp $(SRC_DIR)/fs/File.cpp
IO_SRCS = $(SRC_DIR)/io/ReadPath.cpp $(SRC_DIR)/io/Writeback.cpp $(SRC_DIR)/io/Readahead.cpp
SCHEDULER_SRCS = $(SRC_DIR)/scheduler/IOThreadPool.cpp
METRICS_SRCS = $(SRC_DIR)/metrics/Counters.cpp
API_SRCS = $(SRC_DIR)/api/UserAPI.cpp

LIB_SRCS = $(CACHE_SRCS) $(FS_SRCS) $(IO_SRCS) $(SCHEDULER_SRCS) $(METRICS_SRCS) $(API_SRCS)
LIB_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(LIB_SRCS))

TARGETS = $(BUILD_DIR)/benchmark $(BUILD_DIR)/test_page_cache $(BUILD_DIR)/test_eviction

all: $(TARGETS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/libpagecache.a: $(LIB_OBJS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

$(BUILD_DIR)/benchmark: $(SRC_DIR)/bench/io_benchmark.cpp $(BUILD_DIR)/libpagecache.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_page_cache: $(TEST_DIR)/page_cache_tests.cpp $(BUILD_DIR)/libpagecache.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_eviction: $(TEST_DIR)/eviction_tests.cpp $(BUILD_DIR)/libpagecache.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(BUILD_DIR)/test_page_cache $(BUILD_DIR)/test_eviction
	$(BUILD_DIR)/test_page_cache
	$(BUILD_DIR)/test_eviction

bench: $(BUILD_DIR)/benchmark
	$(BUILD_DIR)/benchmark

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test bench clean
