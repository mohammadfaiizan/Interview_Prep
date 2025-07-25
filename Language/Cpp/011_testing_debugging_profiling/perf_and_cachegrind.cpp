/*
 * TESTING, DEBUGGING, PROFILING - PERF AND CACHEGRIND PROFILING
 * 
 * This file demonstrates performance profiling using Linux perf tools
 * and Valgrind's cachegrind for cache analysis.
 * 
 * Compilation: g++ -std=c++17 -g -O2 perf_and_cachegrind.cpp -o perf_demo
 * 
 * Usage:
 * perf record ./perf_demo
 * perf report
 * valgrind --tool=cachegrind ./perf_demo
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <cstring>

// =============================================================================
// PERFORMANCE MEASUREMENT UTILITIES
// =============================================================================

class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::string operation_name_;
    
public:
    PerformanceTimer(const std::string& name) : operation_name_(name) {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    ~PerformanceTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time_);
        std::cout << operation_name_ << " took: " << duration.count() 
                  << " microseconds" << std::endl;
    }
};

#define PROFILE(name) PerformanceTimer timer(name)

// =============================================================================
// CACHE-FRIENDLY VS CACHE-UNFRIENDLY ALGORITHMS
// =============================================================================

void demonstrate_cache_locality() {
    std::cout << "\n=== Cache Locality Demonstration ===" << std::endl;
    
    const size_t size = 1000;
    std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 1));
    
    // Cache-friendly: Row-major access
    {
        PROFILE("Row-major access (cache-friendly)");
        long long sum = 0;
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                sum += matrix[i][j];
            }
        }
        std::cout << "Sum: " << sum << std::endl;
    }
    
    // Cache-unfriendly: Column-major access
    {
        PROFILE("Column-major access (cache-unfriendly)");
        long long sum = 0;
        for (size_t j = 0; j < size; ++j) {
            for (size_t i = 0; i < size; ++i) {
                sum += matrix[i][j];
            }
        }
        std::cout << "Sum: " << sum << std::endl;
    }
}

void demonstrate_memory_access_patterns() {
    std::cout << "\n=== Memory Access Patterns ===" << std::endl;
    
    const size_t size = 1000000;
    std::vector<int> data(size);
    
    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    
    // Sequential access
    {
        PROFILE("Sequential memory access");
        long long sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += data[i];
        }
        std::cout << "Sequential sum: " << sum << std::endl;
    }
    
    // Random access
    {
        PROFILE("Random memory access");
        std::vector<size_t> indices(size);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gen);
        
        long long sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += data[indices[i]];
        }
        std::cout << "Random sum: " << sum << std::endl;
    }
}

// =============================================================================
// CPU-INTENSIVE ALGORITHMS FOR PROFILING
// =============================================================================

// Fibonacci with memoization
std::vector<long long> fib_memo(100, -1);

long long fibonacci_memo(int n) {
    if (n <= 1) return n;
    if (fib_memo[n] != -1) return fib_memo[n];
    
    fib_memo[n] = fibonacci_memo(n-1) + fibonacci_memo(n-2);
    return fib_memo[n];
}

// Naive fibonacci (intentionally inefficient)
long long fibonacci_naive(int n) {
    if (n <= 1) return n;
    return fibonacci_naive(n-1) + fibonacci_naive(n-2);
}

void demonstrate_algorithm_profiling() {
    std::cout << "\n=== Algorithm Profiling ===" << std::endl;
    
    const int n = 35;
    
    {
        PROFILE("Fibonacci with memoization");
        long long result = fibonacci_memo(n);
        std::cout << "Fibonacci(" << n << ") = " << result << std::endl;
    }
    
    {
        PROFILE("Naive fibonacci (slow)");
        long long result = fibonacci_naive(25);  // Smaller n to avoid long wait
        std::cout << "Fibonacci(25) = " << result << std::endl;
    }
}

// =============================================================================
// SORTING ALGORITHM COMPARISONS
// =============================================================================

void demonstrate_sorting_performance() {
    std::cout << "\n=== Sorting Algorithm Performance ===" << std::endl;
    
    const size_t size = 100000;
    std::vector<int> original_data(size);
    
    // Generate random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);
    
    for (size_t i = 0; i < size; ++i) {
        original_data[i] = dis(gen);
    }
    
    // Test std::sort
    {
        std::vector<int> data = original_data;
        PROFILE("std::sort");
        std::sort(data.begin(), data.end());
    }
    
    // Test std::stable_sort
    {
        std::vector<int> data = original_data;
        PROFILE("std::stable_sort");
        std::stable_sort(data.begin(), data.end());
    }
    
    // Test std::partial_sort (top 1000 elements)
    {
        std::vector<int> data = original_data;
        PROFILE("std::partial_sort (top 1000)");
        std::partial_sort(data.begin(), data.begin() + 1000, data.end());
    }
}

// =============================================================================
// PERF TOOL DEMONSTRATIONS
// =============================================================================

void demonstrate_perf_usage() {
    std::cout << "\n=== Linux Perf Tool Usage ===" << std::endl;
    
    std::cout << "\nBasic perf commands:" << std::endl;
    std::cout << "perf record ./program            # Record performance data" << std::endl;
    std::cout << "perf record -g ./program         # Record with call graphs" << std::endl;
    std::cout << "perf record -F 99 ./program      # Sample at 99 Hz" << std::endl;
    std::cout << "perf report                      # View recorded data" << std::endl;
    std::cout << "perf top                         # Real-time profiling" << std::endl;
    
    std::cout << "\nAdvanced perf usage:" << std::endl;
    std::cout << "perf stat ./program              # Basic performance statistics" << std::endl;
    std::cout << "perf stat -e cache-misses ./program  # Count cache misses" << std::endl;
    std::cout << "perf stat -e instructions,cycles ./program  # IPC analysis" << std::endl;
    std::cout << "perf record -e cache-misses ./program  # Record cache miss events" << std::endl;
    
    std::cout << "\nHardware event monitoring:" << std::endl;
    std::cout << "perf list                        # List available events" << std::endl;
    std::cout << "perf stat -e L1-dcache-load-misses ./program" << std::endl;
    std::cout << "perf stat -e LLC-load-misses ./program" << std::endl;
    std::cout << "perf stat -e branch-misses ./program" << std::endl;
    
    std::cout << "\nFlame graphs (with perf-tools):" << std::endl;
    std::cout << "perf record -F 99 -g ./program" << std::endl;
    std::cout << "perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg" << std::endl;
}

void demonstrate_cachegrind_usage() {
    std::cout << "\n=== Valgrind Cachegrind Usage ===" << std::endl;
    
    std::cout << "\nBasic cachegrind commands:" << std::endl;
    std::cout << "valgrind --tool=cachegrind ./program" << std::endl;
    std::cout << "valgrind --tool=cachegrind --cache-sim=yes ./program" << std::endl;
    std::cout << "valgrind --tool=cachegrind --branch-sim=yes ./program" << std::endl;
    
    std::cout << "\nCachegrind output analysis:" << std::endl;
    std::cout << "cg_annotate cachegrind.out.<pid>        # Annotate source" << std::endl;
    std::cout << "cg_diff cachegrind.out.1 cachegrind.out.2  # Compare runs" << std::endl;
    std::cout << "kcachegrind cachegrind.out.<pid>         # GUI analysis" << std::endl;
    
    std::cout << "\nCachegrind metrics:" << std::endl;
    std::cout << "• Ir: Instruction reads" << std::endl;
    std::cout << "• Dr: Data reads" << std::endl;
    std::cout << "• Dw: Data writes" << std::endl;
    std::cout << "• I1mr: L1 instruction cache misses" << std::endl;
    std::cout << "• D1mr: L1 data cache read misses" << std::endl;
    std::cout << "• D1mw: L1 data cache write misses" << std::endl;
    std::cout << "• ILmr: Last-level instruction cache misses" << std::endl;
    std::cout << "• DLmr: Last-level data cache read misses" << std::endl;
    std::cout << "• DLmw: Last-level data cache write misses" << std::endl;
}

// =============================================================================
// OPTIMIZATION TECHNIQUES BASED ON PROFILING
// =============================================================================

void demonstrate_optimization_techniques() {
    std::cout << "\n=== Optimization Techniques ===" << std::endl;
    
    std::cout << "\n1. Loop Optimization:" << std::endl;
    const size_t size = 1000;
    std::vector<int> vec(size, 1);
    
    // Unoptimized loop
    {
        PROFILE("Unoptimized loop");
        int sum = 0;
        for (size_t i = 0; i < vec.size(); ++i) {
            sum += vec[i] * 2 + 1;  // Complex operation in loop
        }
        std::cout << "Sum: " << sum << std::endl;
    }
    
    // Optimized loop (hoisting invariant)
    {
        PROFILE("Optimized loop");
        int sum = 0;
        const size_t vec_size = vec.size();  // Hoist size calculation
        for (size_t i = 0; i < vec_size; ++i) {
            sum += (vec[i] << 1) + 1;  // Use bit shift instead of multiply
        }
        std::cout << "Sum: " << sum << std::endl;
    }
    
    std::cout << "\n2. Memory Layout Optimization:" << std::endl;
    
    // Array of Structures (AoS) - cache unfriendly for partial access
    struct Point3D_AoS {
        float x, y, z;
    };
    
    // Structure of Arrays (SoA) - cache friendly for single component access
    struct Points3D_SoA {
        std::vector<float> x, y, z;
        
        Points3D_SoA(size_t size) : x(size), y(size), z(size) {}
    };
    
    const size_t num_points = 100000;
    
    // AoS access pattern
    {
        PROFILE("Array of Structures (AoS)");
        std::vector<Point3D_AoS> points(num_points, {1.0f, 2.0f, 3.0f});
        float sum = 0;
        for (const auto& point : points) {
            sum += point.x;  // Only accessing x, but loading y,z too
        }
        std::cout << "AoS sum: " << sum << std::endl;
    }
    
    // SoA access pattern
    {
        PROFILE("Structure of Arrays (SoA)");
        Points3D_SoA points(num_points);
        std::fill(points.x.begin(), points.x.end(), 1.0f);
        std::fill(points.y.begin(), points.y.end(), 2.0f);
        std::fill(points.z.begin(), points.z.end(), 3.0f);
        
        float sum = 0;
        for (float x : points.x) {
            sum += x;  // Only loading x values - better cache usage
        }
        std::cout << "SoA sum: " << sum << std::endl;
    }
}

// =============================================================================
// PROFILING INTEGRATION WITH BUILD SYSTEMS
// =============================================================================

void demonstrate_build_integration() {
    std::cout << "\n=== Build System Integration ===" << std::endl;
    
    std::cout << "\nCMake integration for profiling:" << std::endl;
    std::cout << R"(
# CMakeLists.txt
option(ENABLE_PROFILING "Enable profiling build" OFF)

if(ENABLE_PROFILING)
    # Add profiling flags
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O2 -fno-omit-frame-pointer)
    
    # Custom target for perf profiling
    add_custom_target(profile
        COMMAND perf record -g $<TARGET_FILE:${PROJECT_NAME}>
        COMMAND perf report
        DEPENDS ${PROJECT_NAME}
        COMMENT "Running performance profiling")
    
    # Custom target for cachegrind
    add_custom_target(cachegrind
        COMMAND valgrind --tool=cachegrind $<TARGET_FILE:${PROJECT_NAME}>
        DEPENDS ${PROJECT_NAME}
        COMMENT "Running cache analysis")
endif()
)" << std::endl;

    std::cout << "\nMakefile integration:" << std::endl;
    std::cout << R"(
# Makefile
CXXFLAGS_PROFILE = -g -O2 -fno-omit-frame-pointer

profile: $(TARGET)
	perf record -g ./$(TARGET)
	perf report

cachegrind: $(TARGET)
	valgrind --tool=cachegrind ./$(TARGET)
	cg_annotate cachegrind.out.*

flame: $(TARGET)
	perf record -F 99 -g ./$(TARGET)
	perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
)" << std::endl;
}

int main() {
    std::cout << "PERFORMANCE PROFILING WITH PERF AND CACHEGRIND\n";
    std::cout << "===============================================\n";
    
    demonstrate_cache_locality();
    demonstrate_memory_access_patterns();
    demonstrate_algorithm_profiling();
    demonstrate_sorting_performance();
    demonstrate_perf_usage();
    demonstrate_cachegrind_usage();
    demonstrate_optimization_techniques();
    demonstrate_build_integration();
    
    std::cout << "\nKey Profiling Concepts:" << std::endl;
    std::cout << "• Profile first, optimize second" << std::endl;
    std::cout << "• Focus on hotspots identified by profiler" << std::endl;
    std::cout << "• Cache locality is crucial for performance" << std::endl;
    std::cout << "• Use appropriate tools for different analysis types" << std::endl;
    std::cout << "• Integrate profiling into development workflow" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Profiling is essential for performance optimization
- perf provides comprehensive CPU and hardware event analysis
- cachegrind helps understand cache behavior and memory access patterns
- Cache locality has significant impact on performance
- Different access patterns can dramatically affect performance
- Profiling should be integrated into the development process
- Always measure before and after optimizations
- Focus optimization efforts on actual bottlenecks, not assumptions
*/
