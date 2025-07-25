/*
 * CONCURRENCY AND MULTITHREADING - PARALLEL ALGORITHMS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread -ltbb parallel_algorithms.cpp -o parallel_algorithms
 * Note: May require Intel TBB library for full parallel execution
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
    std::cout << "1. std::execution::seq - Sequential execution" << std::endl;
    std::cout << "2. std::execution::par - Parallel execution" << std::endl;
    std::cout << "3. std::execution::par_unseq - Parallel + vectorized" << std::endl;
    
    std::vector<int> data(1000000);
    std::iota(data.begin(), data.end(), 1);
    
    // Sequential
    auto start = std::chrono::high_resolution_clock::now();
    auto seq_result = std::accumulate(std::execution::seq, 
                                     data.begin(), data.end(), 0LL);
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Parallel (may fall back to sequential if not supported)
    start = std::chrono::high_resolution_clock::now();
    auto par_result = std::reduce(std::execution::par, 
                                 data.begin(), data.end(), 0LL);
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Sequential result: " << seq_result << std::endl;
    std::cout << "Parallel result: " << par_result << std::endl;
    std::cout << "Sequential time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(seq_time).count() 
              << " μs" << std::endl;
    std::cout << "Parallel time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(par_time).count() 
              << " μs" << std::endl;
}

// =============================================================================
// 2. PARALLEL SORTING
// =============================================================================

void demonstrate_parallel_sorting() {
    SECTION("Parallel Sorting");
    
    const size_t size = 1000000;
    std::vector<int> seq_data(size);
    std::vector<int> par_data(size);
    
    // Generate random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    for (size_t i = 0; i < size; ++i) {
        int value = dis(gen);
        seq_data[i] = value;
        par_data[i] = value;
    }
    
    // Sequential sort
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, seq_data.begin(), seq_data.end());
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Parallel sort
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, par_data.begin(), par_data.end());
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    // Verify results are identical
    bool identical = std::equal(seq_data.begin(), seq_data.end(), par_data.begin());
    
    std::cout << "Data size: " << size << " elements" << std::endl;
    std::cout << "Sequential sort: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(seq_time).count() 
              << " ms" << std::endl;
    std::cout << "Parallel sort: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(par_time).count() 
              << " ms" << std::endl;
    std::cout << "Results identical: " << (identical ? "Yes" : "No") << std::endl;
    
    if (par_time < seq_time) {
        double speedup = static_cast<double>(seq_time.count()) / par_time.count();
        std::cout << "Speedup: " << speedup << "x" << std::endl;
    }
}

// =============================================================================
// 3. PARALLEL TRANSFORMATIONS
// =============================================================================

void demonstrate_parallel_transform() {
    SECTION("Parallel Transformations");
    
    const size_t size = 1000000;
    std::vector<double> input(size);
    std::vector<double> seq_output(size);
    std::vector<double> par_output(size);
    
    // Initialize input
    std::iota(input.begin(), input.end(), 1.0);
    
    auto expensive_function = [](double x) {
        return std::sqrt(x) * std::sin(x) * std::cos(x);
    };
    
    // Sequential transform
    auto start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::seq, input.begin(), input.end(), 
                   seq_output.begin(), expensive_function);
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Parallel transform
    start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::par, input.begin(), input.end(), 
                   par_output.begin(), expensive_function);
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Transform operation on " << size << " elements" << std::endl;
    std::cout << "Sequential: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(seq_time).count() 
              << " ms" << std::endl;
    std::cout << "Parallel: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(par_time).count() 
              << " ms" << std::endl;
    
    // Verify first few results
    std::cout << "First 5 results match: ";
    for (int i = 0; i < 5; ++i) {
        if (std::abs(seq_output[i] - par_output[i]) > 1e-10) {
            std::cout << "No" << std::endl;
            return;
        }
    }
    std::cout << "Yes" << std::endl;
}

// =============================================================================
// 4. PARALLEL REDUCTIONS
// =============================================================================

void demonstrate_parallel_reduce() {
    SECTION("Parallel Reductions");
    
    const size_t size = 10000000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    
    // Sequential reduce
    auto start = std::chrono::high_resolution_clock::now();
    long long seq_sum = std::accumulate(data.begin(), data.end(), 0LL);
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Parallel reduce
    start = std::chrono::high_resolution_clock::now();
    long long par_sum = std::reduce(std::execution::par, data.begin(), data.end(), 0LL);
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    // Custom reduction operation
    start = std::chrono::high_resolution_clock::now();
    long long par_product = std::reduce(std::execution::par, data.begin(), data.begin() + 20, 
                                       1LL, std::multiplies<long long>());
    auto product_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Sum of " << size << " elements:" << std::endl;
    std::cout << "Sequential sum: " << seq_sum << std::endl;
    std::cout << "Parallel sum: " << par_sum << std::endl;
    std::cout << "Product of first 20: " << par_product << std::endl;
    
    std::cout << "Sequential time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(seq_time).count() 
              << " μs" << std::endl;
    std::cout << "Parallel time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(par_time).count() 
              << " μs" << std::endl;
}

// =============================================================================
// 5. PARALLEL SEARCH AND FIND
// =============================================================================

void demonstrate_parallel_search() {
    SECTION("Parallel Search and Find");
    
    const size_t size = 10000000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    
    // Add some special values to find
    data[size/2] = 999999;
    data[size/4] = 888888;
    data[3*size/4] = 777777;
    
    int target = 777777;
    
    // Sequential find
    auto start = std::chrono::high_resolution_clock::now();
    auto seq_it = std::find(std::execution::seq, data.begin(), data.end(), target);
    auto seq_time = std::chrono::high_resolution_clock::now() - start;
    
    // Parallel find
    start = std::chrono::high_resolution_clock::now();
    auto par_it = std::find(std::execution::par, data.begin(), data.end(), target);
    auto par_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Searching for " << target << " in " << size << " elements" << std::endl;
    std::cout << "Sequential found at position: " << (seq_it - data.begin()) << std::endl;
    std::cout << "Parallel found at position: " << (par_it - data.begin()) << std::endl;
    
    std::cout << "Sequential time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(seq_time).count() 
              << " μs" << std::endl;
    std::cout << "Parallel time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(par_time).count() 
              << " μs" << std::endl;
    
    // Parallel count_if
    start = std::chrono::high_resolution_clock::now();
    auto even_count = std::count_if(std::execution::par, data.begin(), data.end(),
                                   [](int x) { return x % 2 == 0; });
    auto count_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Even numbers count: " << even_count << std::endl;
    std::cout << "Count time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(count_time).count() 
              << " μs" << std::endl;
}

// =============================================================================
// 6. PERFORMANCE CONSIDERATIONS
// =============================================================================

void demonstrate_performance_considerations() {
    SECTION("Performance Considerations");
    
    std::cout << "When Parallel Algorithms Help:" << std::endl;
    std::cout << "✓ Large datasets (> 10,000 elements typically)" << std::endl;
    std::cout << "✓ CPU-intensive operations" << std::endl;
    std::cout << "✓ Independent computations" << std::endl;
    std::cout << "✓ Multiple CPU cores available" << std::endl;
    
    std::cout << "\nWhen Sequential May Be Better:" << std::endl;
    std::cout << "• Small datasets (overhead dominates)" << std::endl;
    std::cout << "• Memory-bound operations" << std::endl;
    std::cout << "• Operations with dependencies" << std::endl;
    std::cout << "• Single-core systems" << std::endl;
    
    // Demonstrate overhead with small dataset
    std::vector<int> small_data(100);
    std::iota(small_data.begin(), small_data.end(), 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, small_data.begin(), small_data.end());
    auto seq_small = std::chrono::high_resolution_clock::now() - start;
    
    std::iota(small_data.begin(), small_data.end(), 1);
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, small_data.begin(), small_data.end());
    auto par_small = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "\nSmall dataset (100 elements) sort times:" << std::endl;
    std::cout << "Sequential: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(seq_small).count() 
              << " ns" << std::endl;
    std::cout << "Parallel: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(par_small).count() 
              << " ns" << std::endl;
    
    if (seq_small < par_small) {
        std::cout << "Sequential wins due to lower overhead" << std::endl;
    }
}

int main() {
    std::cout << "PARALLEL ALGORITHMS TUTORIAL\n";
    std::cout << "============================\n";
    
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n";
    
    demonstrate_execution_policies();
    demonstrate_parallel_sorting();
    demonstrate_parallel_transform();
    demonstrate_parallel_reduce();
    demonstrate_parallel_search();
    demonstrate_performance_considerations();
    
    return 0;
}

/*
KEY CONCEPTS:
- C++17 execution policies enable parallel algorithms
- std::execution::par provides parallel execution
- Parallel algorithms work best with large datasets
- CPU-intensive operations benefit most from parallelization
- Small datasets may perform worse due to overhead
- Requires compiler and library support (Intel TBB, etc.)
- Always measure performance for your specific use case
- Parallel algorithms are drop-in replacements for sequential ones
*/
