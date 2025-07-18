/*
 * TESTING, DEBUGGING, PROFILING - BENCHMARKING WITH GOOGLE BENCHMARK
 * 
 * This file demonstrates comprehensive benchmarking using Google Benchmark
 * library for performance testing and optimization validation.
 * 
 * Compilation: g++ -std=c++17 -O2 benchmarking_with_googlebench.cpp -lbenchmark -pthread -o benchmark_demo
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>

// Note: In real projects, include <benchmark/benchmark.h>
// For this demonstration, we'll simulate the benchmark interface

// =============================================================================
// MOCK GOOGLE BENCHMARK INTERFACE
// =============================================================================

namespace benchmark {
    class State {
    private:
        bool running_ = true;
        int64_t iterations_ = 0;
        std::chrono::high_resolution_clock::time_point start_time_;
        
    public:
        State() {
            start_time_ = std::chrono::high_resolution_clock::now();
        }
        
        bool KeepRunning() {
            if (iterations_ == 0) {
                start_time_ = std::chrono::high_resolution_clock::now();
            }
            
            iterations_++;
            
            // Simple termination condition for demo
            if (iterations_ > 1000) {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end_time - start_time_);
                
                std::cout << "Benchmark completed: " << iterations_ << " iterations, "
                          << duration.count() / iterations_ << " ns/op" << std::endl;
                running_ = false;
            }
            
            return running_;
        }
        
        void SetBytesProcessed(int64_t bytes) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time_);
            
            double throughput = (bytes * iterations_ * 1e9) / duration.count();
            std::cout << "Throughput: " << throughput / (1024*1024) << " MB/s" << std::endl;
        }
        
        void SetItemsProcessed(int64_t items) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time_);
            
            double rate = (items * iterations_ * 1e9) / duration.count();
            std::cout << "Processing rate: " << rate << " items/s" << std::endl;
        }
        
        void PauseTiming() {
            // In real benchmark, this pauses the timer
        }
        
        void ResumeTiming() {
            // In real benchmark, this resumes the timer
        }
        
        int64_t iterations() const { return iterations_; }
    };
    
    void Initialize(int* argc, char** argv) {
        std::cout << "Benchmark framework initialized" << std::endl;
    }
    
    void RunSpecifiedBenchmarks() {
        std::cout << "Running all benchmarks..." << std::endl;
    }
    
    void RegisterBenchmark(const char* name, void(*func)(State&)) {
        std::cout << "Running benchmark: " << name << std::endl;
        State state;
        func(state);
    }
}

#define BENCHMARK(func) \
    void func(benchmark::State& state); \
    struct func##_registrar { \
        func##_registrar() { \
            benchmark::RegisterBenchmark(#func, func); \
        } \
    }; \
    static func##_registrar func##_instance; \
    void func(benchmark::State& state)

// =============================================================================
// BASIC BENCHMARKING EXAMPLES
// =============================================================================

// Simple function benchmarking
BENCHMARK(BM_StringCreation) {
    while (state.KeepRunning()) {
        std::string empty_string;
    }
}

BENCHMARK(BM_StringCopy) {
    std::string x = "hello world";
    while (state.KeepRunning()) {
        std::string copy(x);
    }
}

// Vector operations benchmarking
BENCHMARK(BM_VectorPushBack) {
    while (state.KeepRunning()) {
        std::vector<int> v;
        v.push_back(42);
    }
}

BENCHMARK(BM_VectorReserveAndPushBack) {
    while (state.KeepRunning()) {
        std::vector<int> v;
        v.reserve(1);
        v.push_back(42);
    }
}

// =============================================================================
// ALGORITHM COMPARISON BENCHMARKS
// =============================================================================

// Sorting algorithm comparisons
void generate_random_data(std::vector<int>& data, size_t size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 10000);
    
    data.resize(size);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
}

BENCHMARK(BM_StdSort_1K) {
    std::vector<int> data;
    generate_random_data(data, 1000);
    
    while (state.KeepRunning()) {
        std::vector<int> copy = data;
        std::sort(copy.begin(), copy.end());
    }
    state.SetItemsProcessed(state.iterations() * 1000);
}

BENCHMARK(BM_StdSort_10K) {
    std::vector<int> data;
    generate_random_data(data, 10000);
    
    while (state.KeepRunning()) {
        std::vector<int> copy = data;
        std::sort(copy.begin(), copy.end());
    }
    state.SetItemsProcessed(state.iterations() * 10000);
}

BENCHMARK(BM_StdStableSort_10K) {
    std::vector<int> data;
    generate_random_data(data, 10000);
    
    while (state.KeepRunning()) {
        std::vector<int> copy = data;
        std::stable_sort(copy.begin(), copy.end());
    }
    state.SetItemsProcessed(state.iterations() * 10000);
}

// =============================================================================
// MEMORY ACCESS PATTERN BENCHMARKS
// =============================================================================

BENCHMARK(BM_SequentialAccess) {
    const size_t size = 1000000;
    std::vector<int> data(size, 1);
    
    while (state.KeepRunning()) {
        long long sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);  // Prevent optimization
    }
    state.SetBytesProcessed(state.iterations() * size * sizeof(int));
}

BENCHMARK(BM_RandomAccess) {
    const size_t size = 1000000;
    std::vector<int> data(size, 1);
    
    // Pre-generate random indices
    std::vector<size_t> indices(size);
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    while (state.KeepRunning()) {
        long long sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += data[indices[i]];
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetBytesProcessed(state.iterations() * size * sizeof(int));
}

// =============================================================================
// STRING PROCESSING BENCHMARKS
// =============================================================================

BENCHMARK(BM_StringConcatenation_Plus) {
    const std::vector<std::string> strings = {
        "Hello", " ", "World", "!", " ", "This", " ", "is", " ", "a", " ", "test"
    };
    
    while (state.KeepRunning()) {
        std::string result;
        for (const auto& s : strings) {
            result += s;
        }
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_StringConcatenation_Stream) {
    const std::vector<std::string> strings = {
        "Hello", " ", "World", "!", " ", "This", " ", "is", " ", "a", " ", "test"
    };
    
    while (state.KeepRunning()) {
        std::ostringstream oss;
        for (const auto& s : strings) {
            oss << s;
        }
        std::string result = oss.str();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_StringConcatenation_Reserve) {
    const std::vector<std::string> strings = {
        "Hello", " ", "World", "!", " ", "This", " ", "is", " ", "a", " ", "test"
    };
    
    while (state.KeepRunning()) {
        std::string result;
        size_t total_size = 0;
        for (const auto& s : strings) {
            total_size += s.length();
        }
        result.reserve(total_size);
        
        for (const auto& s : strings) {
            result += s;
        }
        benchmark::DoNotOptimize(result);
    }
}

// =============================================================================
// ADVANCED BENCHMARKING TECHNIQUES
// =============================================================================

void demonstrate_benchmark_features() {
    std::cout << "\n=== Google Benchmark Features ===" << std::endl;
    
    std::cout << "\n1. Basic Benchmark Macros:" << std::endl;
    std::cout << R"(
BENCHMARK(BM_Function);                    // Simple benchmark
BENCHMARK(BM_Function)->Arg(1000);         // With argument
BENCHMARK(BM_Function)->Range(8, 8<<10);   // Range of arguments
BENCHMARK(BM_Function)->RangeMultiplier(2)->Range(8, 8<<10);
)" << std::endl;

    std::cout << "\n2. Parameterized Benchmarks:" << std::endl;
    std::cout << R"(
static void BM_StringCreation(benchmark::State& state) {
    for (auto _ : state) {
        std::string empty_string;
    }
}
BENCHMARK(BM_StringCreation);

// With custom arguments
BENCHMARK(BM_StringCreation)->Arg(8)->Arg(64)->Arg(512);
BENCHMARK(BM_StringCreation)->Range(8, 8<<10);
BENCHMARK(BM_StringCreation)->RangeMultiplier(2)->Range(8, 8<<10);
)" << std::endl;

    std::cout << "\n3. Multithreaded Benchmarks:" << std::endl;
    std::cout << R"(
BENCHMARK(BM_Function)->Threads(1);
BENCHMARK(BM_Function)->Threads(2);
BENCHMARK(BM_Function)->Threads(4);
BENCHMARK(BM_Function)->Threads(8);
BENCHMARK(BM_Function)->ThreadRange(1, 8);
)" << std::endl;

    std::cout << "\n4. Custom Counters:" << std::endl;
    std::cout << R"(
static void BM_ProcessBytes(benchmark::State& state) {
    std::vector<char> data(state.range(0));
    for (auto _ : state) {
        // Process data
        benchmark::DoNotOptimize(data);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_ProcessBytes)->Range(8, 8<<20);
)" << std::endl;

    std::cout << "\n5. Fixture-based Benchmarks:" << std::endl;
    std::cout << R"(
class MyFixture : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        // Setup code
    }
    
    void TearDown(const ::benchmark::State& state) {
        // Cleanup code
    }
};

BENCHMARK_F(MyFixture, BenchmarkMethod)(benchmark::State& state) {
    for (auto _ : state) {
        // Benchmark code
    }
}
)" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== Benchmarking Best Practices ===" << std::endl;
    
    std::cout << "\n1. Preventing Compiler Optimizations:" << std::endl;
    std::cout << R"(
// Use DoNotOptimize to prevent dead code elimination
benchmark::DoNotOptimize(result);

// Use ClobberMemory to prevent memory optimizations
benchmark::ClobberMemory();

// Example:
static void BM_Example(benchmark::State& state) {
    for (auto _ : state) {
        int result = expensive_computation();
        benchmark::DoNotOptimize(result);  // Prevent optimization
    }
}
)" << std::endl;

    std::cout << "\n2. Setup and Teardown:" << std::endl;
    std::cout << R"(
static void BM_Example(benchmark::State& state) {
    // Setup (outside the timing loop)
    std::vector<int> data(state.range(0));
    
    for (auto _ : state) {
        // Only the code inside this loop is timed
        std::sort(data.begin(), data.end());
        
        // Reset data for next iteration
        state.PauseTiming();
        std::random_shuffle(data.begin(), data.end());
        state.ResumeTiming();
    }
}
)" << std::endl;

    std::cout << "\n3. Realistic Test Data:" << std::endl;
    std::cout << "• Use representative data sizes" << std::endl;
    std::cout << "• Include edge cases (empty, very large)" << std::endl;
    std::cout << "• Use realistic data distributions" << std::endl;
    std::cout << "• Consider cache effects" << std::endl;
    
    std::cout << "\n4. Statistical Considerations:" << std::endl;
    std::cout << "• Run benchmarks multiple times" << std::endl;
    std::cout << "• Use sufficient iterations for stable results" << std::endl;
    std::cout << "• Consider system load and background processes" << std::endl;
    std::cout << "• Use CPU affinity for consistent results" << std::endl;
    
    std::cout << "\n5. Benchmark Organization:" << std::endl;
    std::cout << R"(
// Group related benchmarks
BENCHMARK(BM_Sort_StdSort)->Range(8, 8<<15);
BENCHMARK(BM_Sort_StableSort)->Range(8, 8<<15);
BENCHMARK(BM_Sort_PartialSort)->Range(8, 8<<15);

// Use meaningful names
BENCHMARK(BM_HashTable_Insert_Sequential);
BENCHMARK(BM_HashTable_Insert_Random);
BENCHMARK(BM_HashTable_Lookup_Hit);
BENCHMARK(BM_HashTable_Lookup_Miss);
)" << std::endl;
}

void demonstrate_cmake_integration() {
    std::cout << "\n=== CMake Integration ===" << std::endl;
    
    std::cout << R"(
# CMakeLists.txt
find_package(benchmark REQUIRED)

# Create benchmark executable
add_executable(my_benchmarks
    benchmark_main.cpp
    algorithm_benchmarks.cpp
    container_benchmarks.cpp)

# Link Google Benchmark
target_link_libraries(my_benchmarks
    PRIVATE
        benchmark::benchmark
        my_library_to_benchmark)

# Enable optimizations for benchmarks
target_compile_options(my_benchmarks PRIVATE -O2 -DNDEBUG)

# Custom target to run benchmarks
add_custom_target(run_benchmarks
    COMMAND my_benchmarks --benchmark_format=json --benchmark_out=results.json
    DEPENDS my_benchmarks
    COMMENT "Running benchmarks")

# Benchmark comparison target
add_custom_target(compare_benchmarks
    COMMAND python3 compare.py baseline.json results.json
    DEPENDS run_benchmarks
    COMMENT "Comparing benchmark results")
)" << std::endl;
}

void demonstrate_ci_integration() {
    std::cout << "\n=== CI/CD Integration ===" << std::endl;
    
    std::cout << "\nGitHub Actions example:" << std::endl;
    std::cout << R"(
name: Benchmarks
on: [push, pull_request]

jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y libbenchmark-dev
    
    - name: Build benchmarks
      run: |
        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make benchmarks
    
    - name: Run benchmarks
      run: |
        cd build
        ./benchmarks --benchmark_format=json --benchmark_out=results.json
    
    - name: Compare with baseline
      run: |
        python3 tools/compare_benchmarks.py baseline.json build/results.json
)" << std::endl;
}

// Mock DoNotOptimize function
namespace benchmark {
    template<typename T>
    void DoNotOptimize(T&& value) {
        // In real benchmark library, this prevents optimization
        volatile auto temp = value;
        (void)temp;
    }
    
    void ClobberMemory() {
        // In real benchmark library, this prevents memory optimizations
        asm volatile("" : : : "memory");
    }
}

int main() {
    std::cout << "BENCHMARKING WITH GOOGLE BENCHMARK\n";
    std::cout << "===================================\n";
    
    // In real code, you would call:
    // benchmark::Initialize(&argc, argv);
    // benchmark::RunSpecifiedBenchmarks();
    
    demonstrate_benchmark_features();
    demonstrate_best_practices();
    demonstrate_cmake_integration();
    demonstrate_ci_integration();
    
    std::cout << "\nKey Benchmarking Concepts:" << std::endl;
    std::cout << "• Measure performance objectively with statistical rigor" << std::endl;
    std::cout << "• Prevent compiler optimizations that invalidate results" << std::endl;
    std::cout << "• Use realistic test data and scenarios" << std::endl;
    std::cout << "• Compare alternatives systematically" << std::endl;
    std::cout << "• Integrate benchmarking into development workflow" << std::endl;
    std::cout << "• Track performance regressions over time" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Google Benchmark provides comprehensive performance measurement
- Proper benchmark design prevents misleading results
- Compiler optimization prevention is crucial for accurate measurements
- Statistical rigor ensures reliable performance comparisons
- Integration with CI/CD enables continuous performance monitoring
- Benchmarking should be part of the development process
- Performance regression detection helps maintain code quality
- Systematic comparison of alternatives guides optimization decisions
*/
