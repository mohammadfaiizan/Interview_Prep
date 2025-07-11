/*
 * MEMORY AND POINTERS - PARALLEL ALGORITHMS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread parallel_algorithms.cpp -o parallel_algorithms
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include <numeric>
#include <chrono>
#include <random>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. EXECUTION POLICIES
// =============================================================================

void demonstrate_execution_policies() {
    SECTION("Execution Policies");
    
    std::cout << "C++17 Execution Policies:" << std::endl;
    std::cout << "- std::execution::seq: Sequential execution" << std::endl;
    std::cout << "- std::execution::par: Parallel execution" << std::endl;
    std::cout << "- std::execution::par_unseq: Parallel and vectorized" << std::endl;
    
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 1);
    
    // Sequential
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, data.begin(), data.end(), std::greater<int>());
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Shuffle for next test
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data.begin(), data.end(), g);
    
    // Parallel
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, data.begin(), data.end(), std::greater<int>());
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Sequential time: " << std::chrono::duration_cast<std::chrono::microseconds>(seq_time).count() << " μs" << std::endl;
    std::cout << "Parallel time: " << std::chrono::duration_cast<std::chrono::microseconds>(par_time).count() << " μs" << std::endl;
}

// =============================================================================
// 2. PARALLEL ALGORITHMS
// =============================================================================

void demonstrate_parallel_algorithms() {
    SECTION("Parallel Algorithms");
    
    const size_t size = 100000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    
    // Parallel for_each
    std::for_each(std::execution::par, data.begin(), data.end(), [](int& x) {
        x = x * x;  // Square each element
    });
    
    std::cout << "First 10 squared values: ";
    for (size_t i = 0; i < 10; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
    
    // Parallel transform
    std::vector<int> results(size);
    std::transform(std::execution::par, data.begin(), data.end(), results.begin(), [](int x) {
        return x / 2;
    });
    
    // Parallel reduce
    int sum = std::reduce(std::execution::par, results.begin(), results.end(), 0);
    std::cout << "Parallel sum: " << sum << std::endl;
    
    // Parallel find
    auto it = std::find(std::execution::par, data.begin(), data.end(), 100);
    if (it != data.end()) {
        std::cout << "Found 100 at position: " << std::distance(data.begin(), it) << std::endl;
    }
}

// =============================================================================
// 3. PERFORMANCE COMPARISON
// =============================================================================

template<typename Policy>
auto time_algorithm(Policy&& policy, const std::vector<int>& data, const std::string& name) {
    auto copy = data;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::forward<Policy>(policy), copy.begin(), copy.end());
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << name << " sort time: " << duration.count() << " μs" << std::endl;
    
    return duration;
}

void demonstrate_performance_comparison() {
    SECTION("Performance Comparison");
    
    const size_t size = 1000000;
    std::vector<int> data(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
    
    auto seq_time = time_algorithm(std::execution::seq, data, "Sequential");
    auto par_time = time_algorithm(std::execution::par, data, "Parallel");
    
    double speedup = static_cast<double>(seq_time.count()) / par_time.count();
    std::cout << "Speedup: " << speedup << "x" << std::endl;
}

// =============================================================================
// 4. CUSTOM PARALLEL OPERATIONS
// =============================================================================

// Simple parallel map implementation
template<typename Iterator, typename Function>
void parallel_map(Iterator first, Iterator last, Function func) {
    const size_t length = std::distance(first, last);
    const size_t num_threads = std::thread::hardware_concurrency();
    
    if (length < num_threads * 2) {
        // Use sequential for small datasets
        std::for_each(first, last, func);
        return;
    }
    
    const size_t chunk_size = length / num_threads;
    std::vector<std::thread> threads;
    
    auto chunk_start = first;
    for (size_t i = 0; i < num_threads - 1; ++i) {
        auto chunk_end = chunk_start;
        std::advance(chunk_end, chunk_size);
        
        threads.emplace_back([chunk_start, chunk_end, func]() {
            std::for_each(chunk_start, chunk_end, func);
        });
        
        chunk_start = chunk_end;
    }
    
    // Handle remaining elements in main thread
    std::for_each(chunk_start, last, func);
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
}

void demonstrate_custom_parallel() {
    SECTION("Custom Parallel Operations");
    
    std::vector<int> data(100000);
    std::iota(data.begin(), data.end(), 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    parallel_map(data.begin(), data.end(), [](int& x) {
        x = x * 2 + 1;  // Some computation
    });
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Custom parallel map time: " << duration.count() << " μs" << std::endl;
    std::cout << "First 10 results: ";
    for (size_t i = 0; i < 10; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. THREAD-SAFE OPERATIONS
// =============================================================================

void demonstrate_thread_safety() {
    SECTION("Thread Safety in Parallel Algorithms");
    
    std::cout << "Thread safety considerations:" << std::endl;
    std::cout << "- Parallel algorithms may use multiple threads" << std::endl;
    std::cout << "- Operations must be thread-safe" << std::endl;
    std::cout << "- Avoid shared mutable state" << std::endl;
    std::cout << "- Use atomic operations for counters" << std::endl;
    
    std::vector<int> data(10000);
    std::iota(data.begin(), data.end(), 1);
    
    std::atomic<int> counter{0};
    
    // Thread-safe parallel operation
    std::for_each(std::execution::par, data.begin(), data.end(), [&counter](int& x) {
        if (x % 2 == 0) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
        x *= 2;
    });
    
    std::cout << "Even numbers processed: " << counter.load() << std::endl;
}

// =============================================================================
// 6. BEST PRACTICES
// =============================================================================

void demonstrate_best_practices() {
    SECTION("Best Practices");
    
    std::cout << "Parallel Algorithm Best Practices:" << std::endl;
    std::cout << "1. Use parallel algorithms for large datasets" << std::endl;
    std::cout << "2. Ensure operations are thread-safe" << std::endl;
    std::cout << "3. Avoid false sharing in data structures" << std::endl;
    std::cout << "4. Consider memory bandwidth limitations" << std::endl;
    std::cout << "5. Profile to verify performance gains" << std::endl;
    std::cout << "6. Use appropriate execution policy" << std::endl;
    
    // Example: When to use parallel algorithms
    const size_t small_size = 100;
    const size_t large_size = 100000;
    
    std::vector<int> small_data(small_size);
    std::vector<int> large_data(large_size);
    
    std::iota(small_data.begin(), small_data.end(), 1);
    std::iota(large_data.begin(), large_data.end(), 1);
    
    // For small datasets, sequential might be faster
    std::sort(std::execution::seq, small_data.begin(), small_data.end());
    
    // For large datasets, parallel is likely faster
    std::sort(std::execution::par, large_data.begin(), large_data.end());
    
    std::cout << "Choose execution policy based on data size and complexity" << std::endl;
}

int main() {
    std::cout << "PARALLEL ALGORITHMS TUTORIAL\n";
    std::cout << "============================\n";
    
    demonstrate_execution_policies();
    demonstrate_parallel_algorithms();
    demonstrate_performance_comparison();
    demonstrate_custom_parallel();
    demonstrate_thread_safety();
    demonstrate_best_practices();
    
    return 0;
}

/*
KEY CONCEPTS:
- C++17 introduces parallel execution policies
- std::execution::par enables parallel algorithm execution
- Parallel algorithms can provide significant speedup
- Operations must be thread-safe in parallel context
- Custom parallel implementations using std::thread
- Performance gains depend on data size and complexity
- Always profile to verify parallel benefits
- Consider memory bandwidth and false sharing
*/
