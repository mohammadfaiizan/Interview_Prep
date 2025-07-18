/*
 * CONCURRENCY AND MULTITHREADING - COROUTINES VS THREADS
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra -pthread coroutines_vs_threads.cpp -o coroutines_vs_threads
 */

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. THREAD APPROACH
// =============================================================================

void cpu_intensive_task(int id, int iterations) {
    long sum = 0;
    for (int i = 0; i < iterations; ++i) {
        sum += i * i;
    }
    std::cout << "Thread " << id << " computed sum: " << sum << std::endl;
}

void io_simulation_task(int id) {
    std::cout << "Thread " << id << " starting I/O operation" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Thread " << id << " I/O operation completed" << std::endl;
}

void demonstrate_threads() {
    SECTION("Thread Approach");
    
    // CPU-intensive tasks
    std::vector<std::thread> cpu_threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 4; ++i) {
        cpu_threads.emplace_back(cpu_intensive_task, i, 1000000);
    }
    
    for (auto& t : cpu_threads) {
        t.join();
    }
    
    auto cpu_time = std::chrono::high_resolution_clock::now() - start;
    std::cout << "CPU threads time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(cpu_time).count() 
              << "ms" << std::endl;
    
    // I/O simulation
    std::vector<std::thread> io_threads;
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 10; ++i) {
        io_threads.emplace_back(io_simulation_task, i);
    }
    
    for (auto& t : io_threads) {
        t.join();
    }
    
    auto io_time = std::chrono::high_resolution_clock::now() - start;
    std::cout << "I/O threads time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(io_time).count() 
              << "ms" << std::endl;
}

// =============================================================================
// 2. COMPARISON ANALYSIS
// =============================================================================

void demonstrate_comparison() {
    SECTION("Threads vs Coroutines Comparison");
    
    std::cout << "THREADS:" << std::endl;
    std::cout << "Advantages:" << std::endl;
    std::cout << "- True parallelism on multiple cores" << std::endl;
    std::cout << "- Preemptive scheduling" << std::endl;
    std::cout << "- Well-established ecosystem" << std::endl;
    std::cout << "- Excellent for CPU-bound tasks" << std::endl;
    
    std::cout << "\nDisadvantages:" << std::endl;
    std::cout << "- High memory overhead (~8KB stack)" << std::endl;
    std::cout << "- Expensive context switching" << std::endl;
    std::cout << "- Limited scalability" << std::endl;
    std::cout << "- Complex synchronization" << std::endl;
    
    std::cout << "\nCOROUTINES:" << std::endl;
    std::cout << "Advantages:" << std::endl;
    std::cout << "- Minimal memory overhead (~100 bytes)" << std::endl;
    std::cout << "- Cheap context switching" << std::endl;
    std::cout << "- Massive scalability (millions)" << std::endl;
    std::cout << "- Cooperative scheduling" << std::endl;
    
    std::cout << "\nDisadvantages:" << std::endl;
    std::cout << "- Single-threaded by default" << std::endl;
    std::cout << "- Requires C++20" << std::endl;
    std::cout << "- Must yield explicitly" << std::endl;
    std::cout << "- Complex implementation" << std::endl;
}

// =============================================================================
// 3. RESOURCE USAGE
// =============================================================================

void demonstrate_resource_usage() {
    SECTION("Resource Usage Analysis");
    
    std::cout << "Memory Usage per Task:" << std::endl;
    std::cout << "Thread: ~8KB (stack) + ~1KB (control block)" << std::endl;
    std::cout << "Coroutine: ~100-200 bytes typical" << std::endl;
    
    std::cout << "\nCreation Overhead:" << std::endl;
    std::cout << "Thread: System call, kernel involvement" << std::endl;
    std::cout << "Coroutine: Function call, user space" << std::endl;
    
    std::cout << "\nContext Switch Cost:" << std::endl;
    std::cout << "Thread: ~1-10 microseconds" << std::endl;
    std::cout << "Coroutine: ~10-100 nanoseconds" << std::endl;
    
    // Demonstrate thread creation overhead
    const int num_tasks = 100;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_tasks; ++i) {
        threads.emplace_back([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto thread_time = std::chrono::high_resolution_clock::now() - start;
    std::cout << "\nThread creation/execution time for " << num_tasks << " tasks: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(thread_time).count()
              << "ms" << std::endl;
}

// =============================================================================
// 4. USE CASE GUIDELINES
// =============================================================================

void demonstrate_use_cases() {
    SECTION("When to Use Each");
    
    std::cout << "USE THREADS FOR:" << std::endl;
    std::cout << "✓ CPU-intensive computations" << std::endl;
    std::cout << "✓ Parallel algorithms" << std::endl;
    std::cout << "✓ Mathematical processing" << std::endl;
    std::cout << "✓ Image/video processing" << std::endl;
    std::cout << "✓ Scientific simulations" << std::endl;
    
    std::cout << "\nUSE COROUTINES FOR:" << std::endl;
    std::cout << "✓ I/O-bound operations" << std::endl;
    std::cout << "✓ Network programming" << std::endl;
    std::cout << "✓ Web servers" << std::endl;
    std::cout << "✓ Event-driven systems" << std::endl;
    std::cout << "✓ Asynchronous APIs" << std::endl;
    
    std::cout << "\nHYBRID APPROACHES:" << std::endl;
    std::cout << "• Thread pool for CPU work + coroutines for coordination" << std::endl;
    std::cout << "• Coroutines for I/O + threads for parallel processing" << std::endl;
    std::cout << "• Actor model with thread-per-actor + coroutine-per-message" << std::endl;
}

// =============================================================================
// 5. PERFORMANCE CHARACTERISTICS
// =============================================================================

void demonstrate_performance() {
    SECTION("Performance Characteristics");
    
    std::cout << "Scalability Limits:" << std::endl;
    std::cout << "Threads: ~1,000-10,000 (OS dependent)" << std::endl;
    std::cout << "Coroutines: ~1,000,000+ (memory dependent)" << std::endl;
    
    std::cout << "\nLatency Characteristics:" << std::endl;
    std::cout << "Thread creation: ~50-500 microseconds" << std::endl;
    std::cout << "Coroutine creation: ~10-100 nanoseconds" << std::endl;
    
    std::cout << "\nThroughput Considerations:" << std::endl;
    std::cout << "Threads: Limited by core count for CPU tasks" << std::endl;
    std::cout << "Coroutines: Limited by single-core performance" << std::endl;
    
    // Simple scalability test
    const int small_count = 10;
    const int large_count = 100;
    
    // Small number of threads
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> small_threads;
    
    for (int i = 0; i < small_count; ++i) {
        small_threads.emplace_back([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    for (auto& t : small_threads) {
        t.join();
    }
    
    auto small_time = std::chrono::high_resolution_clock::now() - start;
    
    // Larger number of threads
    start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> large_threads;
    
    for (int i = 0; i < large_count; ++i) {
        large_threads.emplace_back([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    for (auto& t : large_threads) {
        t.join();
    }
    
    auto large_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "\nScalability Test:" << std::endl;
    std::cout << small_count << " threads: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(small_time).count() << "ms" << std::endl;
    std::cout << large_count << " threads: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(large_time).count() << "ms" << std::endl;
}

// =============================================================================
// 6. FUTURE DIRECTIONS
// =============================================================================

void demonstrate_future_directions() {
    SECTION("Future Directions");
    
    std::cout << "C++20 Coroutines Status:" << std::endl;
    std::cout << "• Low-level infrastructure provided" << std::endl;
    std::cout << "• Requires library implementations" << std::endl;
    std::cout << "• Complex to use directly" << std::endl;
    
    std::cout << "\nUpcoming Improvements:" << std::endl;
    std::cout << "• Higher-level async/await syntax" << std::endl;
    std::cout << "• Standard library coroutine utilities" << std::endl;
    std::cout << "• Better integration with existing APIs" << std::endl;
    
    std::cout << "\nCurrent Alternatives:" << std::endl;
    std::cout << "• std::async + std::future" << std::endl;
    std::cout << "• Callback-based async programming" << std::endl;
    std::cout << "• Third-party coroutine libraries" << std::endl;
    
    std::cout << "\nBest Practices:" << std::endl;
    std::cout << "• Use threads for CPU-bound work" << std::endl;
    std::cout << "• Consider coroutines for I/O-bound work" << std::endl;
    std::cout << "• Profile your specific use case" << std::endl;
    std::cout << "• Don't over-engineer for theoretical performance" << std::endl;
}

int main() {
    std::cout << "COROUTINES VS THREADS TUTORIAL\n";
    std::cout << "==============================\n";
    
    demonstrate_threads();
    demonstrate_comparison();
    demonstrate_resource_usage();
    demonstrate_use_cases();
    demonstrate_performance();
    demonstrate_future_directions();
    
    std::cout << "\nHardware info:" << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Threads: true parallelism, high overhead, CPU-bound tasks
- Coroutines: cooperative multitasking, low overhead, I/O-bound tasks
- Memory: threads ~8KB, coroutines ~100 bytes
- Scalability: threads thousands, coroutines millions
- Performance depends on workload characteristics
- Hybrid approaches often optimal
- C++20 provides low-level coroutine support
- Choose based on specific use case requirements
*/
