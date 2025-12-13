#include <iostream>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <cstring>
#include "cache/PageCache.h"
#include "fs/File.h"
#include "api/UserAPI.h"

using namespace pagecache;
using namespace std::chrono;

class Benchmark
{
public:
    struct Result
    {
        std::string name;
        double throughput_mbps;
        double latency_us;
        uint64_t operations;
        double hit_ratio;
    };

    static void setup_test_file(const std::string &filename, size_t size_mb)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            std::cerr << "Failed to create test file: " << filename << std::endl;
            return;
        }

        size_t size_bytes = size_mb * 1024 * 1024;
        char buffer[4096];
        std::memset(buffer, 'a', sizeof(buffer));

        for (size_t i = 0; i < size_bytes; i += sizeof(buffer))
        {
            file.write(buffer, sizeof(buffer));
        }
        file.close();
    }

    static Result sequential_read_benchmark(const std::string &filename, size_t iterations)
    {
        auto &sys = PageCacheSystem::instance();
        sys.set_eviction_policy("lru");

        auto start = high_resolution_clock::now();

        uint64_t total_bytes = 0;
        for (size_t i = 0; i < iterations; ++i)
        {
            auto file = sys.open_file(filename, FileMode::ReadOnly);
            uint8_t buffer[8192];

            size_t bytes_read = file->read(buffer, sizeof(buffer));
            total_bytes += bytes_read;

            sys.close_file(file);
        }

        auto end = high_resolution_clock::now();
        auto duration_ms = duration_cast<milliseconds>(end - start).count();

        double throughput = (total_bytes / (1024.0 * 1024.0)) / (duration_ms / 1000.0);
        double latency = (duration_ms * 1000.0) / iterations;

        return {
            "Sequential Read",
            throughput,
            latency,
            iterations,
            sys.get_counters()->hit_ratio()};
    }

    static Result random_read_benchmark(const std::string &filename, size_t iterations)
    {
        auto &sys = PageCacheSystem::instance();
        sys.set_eviction_policy("lru");

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(0, 100000);

        auto start = high_resolution_clock::now();

        uint64_t total_bytes = 0;
        for (size_t i = 0; i < iterations; ++i)
        {
            auto file = sys.open_file(filename, FileMode::ReadOnly);
            uint64_t offset = dist(rng) * 4096;
            file->seek(offset);

            uint8_t buffer[4096];
            size_t bytes_read = file->read(buffer, sizeof(buffer));
            total_bytes += bytes_read;

            sys.close_file(file);
        }

        auto end = high_resolution_clock::now();
        auto duration_ms = duration_cast<milliseconds>(end - start).count();

        double throughput = (total_bytes / (1024.0 * 1024.0)) / (duration_ms / 1000.0);
        double latency = (duration_ms * 1000.0) / iterations;

        return {
            "Random Read",
            throughput,
            latency,
            iterations,
            sys.get_counters()->hit_ratio()};
    }

    static Result mixed_workload_benchmark(const std::string &filename, size_t iterations)
    {
        auto &sys = PageCacheSystem::instance();
        sys.set_eviction_policy("lru");

        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        auto start = high_resolution_clock::now();

        uint64_t total_bytes = 0;
        for (size_t i = 0; i < iterations; ++i)
        {
            double op = dist(rng);

            if (op < 0.7)
            {
                auto file = sys.open_file(filename, FileMode::ReadOnly);
                uint8_t buffer[4096];
                size_t bytes_read = file->read(buffer, sizeof(buffer));
                total_bytes += bytes_read;
                sys.close_file(file);
            }
            else
            {
                auto file = sys.open_file(filename, FileMode::ReadWrite);
                uint8_t buffer[4096];
                std::memset(buffer, 'x', sizeof(buffer));
                size_t bytes_written = file->write(buffer, sizeof(buffer));
                total_bytes += bytes_written;
                sys.close_file(file);
            }
        }

        auto end = high_resolution_clock::now();
        auto duration_ms = duration_cast<milliseconds>(end - start).count();

        double throughput = (total_bytes / (1024.0 * 1024.0)) / (duration_ms / 1000.0);
        double latency = (duration_ms * 1000.0) / iterations;

        return {
            "Mixed Read/Write",
            throughput,
            latency,
            iterations,
            sys.get_counters()->hit_ratio()};
    }

    static void print_result(const Result &r)
    {
        std::cout << std::left << std::setw(25) << r.name
                  << std::setw(15) << std::fixed << std::setprecision(2) << r.throughput_mbps << " MB/s"
                  << std::setw(15) << std::fixed << std::setprecision(3) << r.latency_us << " us"
                  << std::setw(15) << std::fixed << std::setprecision(3) << (r.hit_ratio * 100) << " %"
                  << std::endl;
    }

    static void print_csv_header()
    {
        std::cout << "Benchmark,Throughput (MB/s),Latency (us),Operations,Hit Ratio (%)" << std::endl;
    }

    static void print_csv_result(const Result &r)
    {
        std::cout << r.name << ","
                  << std::fixed << std::setprecision(2) << r.throughput_mbps << ","
                  << std::fixed << std::setprecision(3) << r.latency_us << ","
                  << r.operations << ","
                  << std::fixed << std::setprecision(3) << (r.hit_ratio * 100) << std::endl;
    }
};

int main(int argc, char **argv)
{
    std::string test_file = "/tmp/pagecache_test.dat";
    size_t file_size_mb = 256;

    std::cout << "PageCache Benchmark Suite" << std::endl;
    std::cout << "==========================" << std::endl;

    std::cout << "\nSetting up test file (" << file_size_mb << " MB)..." << std::endl;
    Benchmark::setup_test_file(test_file, file_size_mb);

    std::cout << "\nRunning benchmarks...\n"
              << std::endl;
    std::cout << std::left << std::setw(25) << "Benchmark"
              << std::setw(15) << "Throughput"
              << std::setw(15) << "Latency"
              << std::setw(15) << "Hit Ratio"
              << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    auto seq_result = Benchmark::sequential_read_benchmark(test_file, 1000);
    Benchmark::print_result(seq_result);

    auto rand_result = Benchmark::random_read_benchmark(test_file, 500);
    Benchmark::print_result(rand_result);

    auto mixed_result = Benchmark::mixed_workload_benchmark(test_file, 800);
    Benchmark::print_result(mixed_result);

    std::cout << "\nCSV Output:" << std::endl;
    Benchmark::print_csv_header();
    Benchmark::print_csv_result(seq_result);
    Benchmark::print_csv_result(rand_result);
    Benchmark::print_csv_result(mixed_result);

    return 0;
}
