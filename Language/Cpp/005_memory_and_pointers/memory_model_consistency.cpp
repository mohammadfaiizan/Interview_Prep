/*
 * MEMORY AND POINTERS - MEMORY MODEL AND CONSISTENCY
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread memory_model_consistency.cpp -o memory_model
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. MEMORY ORDERING BASICS
// =============================================================================

void demonstrate_memory_ordering_basics() {
    SECTION("Memory Ordering Basics");
    
    std::cout << "C++ Memory Ordering Options:" << std::endl;
    std::cout << "- memory_order_relaxed: No synchronization" << std::endl;
    std::cout << "- memory_order_acquire: Acquire operation" << std::endl;
    std::cout << "- memory_order_release: Release operation" << std::endl;
    std::cout << "- memory_order_acq_rel: Both acquire and release" << std::endl;
    std::cout << "- memory_order_seq_cst: Sequential consistency (default)" << std::endl;
    std::cout << "- memory_order_consume: Consume operation (deprecated)" << std::endl;
    
    std::atomic<int> counter{0};
    
    // Different memory orderings
    counter.store(1, std::memory_order_relaxed);
    counter.store(2, std::memory_order_release);
    counter.store(3, std::memory_order_seq_cst);
    
    int val1 = counter.load(std::memory_order_relaxed);
    int val2 = counter.load(std::memory_order_acquire);
    int val3 = counter.load(std::memory_order_seq_cst);
    
    std::cout << "Values: " << val1 << ", " << val2 << ", " << val3 << std::endl;
}

// =============================================================================
// 2. RELAXED ORDERING
// =============================================================================

std::atomic<int> relaxed_counter{0};
std::atomic<int> relaxed_data[5];

void relaxed_writer(int id) {
    for (int i = 0; i < 1000; ++i) {
        relaxed_data[id].store(i, std::memory_order_relaxed);
        relaxed_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void demonstrate_relaxed_ordering() {
    SECTION("Relaxed Memory Ordering");
    
    // Initialize
    relaxed_counter.store(0);
    for (int i = 0; i < 5; ++i) {
        relaxed_data[i].store(0);
    }
    
    std::vector<std::thread> writers;
    for (int i = 0; i < 3; ++i) {
        writers.emplace_back(relaxed_writer, i);
    }
    
    for (auto& writer : writers) {
        writer.join();
    }
    
    std::cout << "Relaxed counter final value: " << relaxed_counter.load() << std::endl;
    std::cout << "Data values: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << relaxed_data[i].load() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Relaxed ordering: No synchronization between threads" << std::endl;
}

// =============================================================================
// 3. ACQUIRE-RELEASE ORDERING
// =============================================================================

std::atomic<bool> ready{false};
std::atomic<int> payload{0};

void acquire_release_producer() {
    payload.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);  // Release: synchronizes with acquire
}

void acquire_release_consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire: synchronizes with release
        std::this_thread::yield();
    }
    
    // Guaranteed to see payload = 42 due to acquire-release synchronization
    int data = payload.load(std::memory_order_relaxed);
    std::cout << "Consumer received payload: " << data << std::endl;
}

void demonstrate_acquire_release() {
    SECTION("Acquire-Release Ordering");
    
    ready.store(false);
    payload.store(0);
    
    std::thread producer(acquire_release_producer);
    std::thread consumer(acquire_release_consumer);
    
    producer.join();
    consumer.join();
    
    std::cout << "Acquire-release ensures proper synchronization" << std::endl;
}

// =============================================================================
// 4. SEQUENTIAL CONSISTENCY
// =============================================================================

std::atomic<bool> x{false}, y{false};
std::atomic<int> z{0};

void seq_cst_thread1() {
    x.store(true, std::memory_order_seq_cst);
    if (y.load(std::memory_order_seq_cst)) {
        z.fetch_add(1, std::memory_order_seq_cst);
    }
}

void seq_cst_thread2() {
    y.store(true, std::memory_order_seq_cst);
    if (x.load(std::memory_order_seq_cst)) {
        z.fetch_add(1, std::memory_order_seq_cst);
    }
}

void demonstrate_sequential_consistency() {
    SECTION("Sequential Consistency");
    
    // Run multiple times to see different outcomes
    for (int run = 0; run < 5; ++run) {
        x.store(false);
        y.store(false);
        z.store(0);
        
        std::thread t1(seq_cst_thread1);
        std::thread t2(seq_cst_thread2);
        
        t1.join();
        t2.join();
        
        std::cout << "Run " << run << ": z = " << z.load() << std::endl;
    }
    
    std::cout << "Sequential consistency provides total ordering" << std::endl;
}

// =============================================================================
// 5. MEMORY FENCES
// =============================================================================

std::atomic<int> fence_data{0};
std::atomic<bool> fence_flag{false};

void fence_producer() {
    fence_data.store(100, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);  // Release fence
    fence_flag.store(true, std::memory_order_relaxed);
}

void fence_consumer() {
    while (!fence_flag.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
    std::atomic_thread_fence(std::memory_order_acquire);   // Acquire fence
    int value = fence_data.load(std::memory_order_relaxed);
    std::cout << "Fence consumer got: " << value << std::endl;
}

void demonstrate_memory_fences() {
    SECTION("Memory Fences");
    
    fence_data.store(0);
    fence_flag.store(false);
    
    std::thread producer(fence_producer);
    std::thread consumer(fence_consumer);
    
    producer.join();
    consumer.join();
    
    std::cout << "Memory fences provide synchronization without atomic operations" << std::endl;
}

// =============================================================================
// 6. COMPARE AND SWAP MEMORY ORDERING
// =============================================================================

std::atomic<int> cas_counter{0};

void cas_increment_relaxed() {
    int expected = cas_counter.load(std::memory_order_relaxed);
    while (!cas_counter.compare_exchange_weak(expected, expected + 1, 
                                             std::memory_order_relaxed)) {
        // Retry with updated expected value
    }
}

void cas_increment_seq_cst() {
    int expected = cas_counter.load();
    while (!cas_counter.compare_exchange_weak(expected, expected + 1)) {
        // Default is sequential consistency
    }
}

void demonstrate_cas_memory_ordering() {
    SECTION("Compare-and-Swap Memory Ordering");
    
    // Test relaxed CAS
    cas_counter.store(0);
    std::vector<std::thread> relaxed_threads;
    
    for (int i = 0; i < 4; ++i) {
        relaxed_threads.emplace_back([]() {
            for (int j = 0; j < 250; ++j) {
                cas_increment_relaxed();
            }
        });
    }
    
    for (auto& t : relaxed_threads) {
        t.join();
    }
    
    std::cout << "Relaxed CAS result: " << cas_counter.load() << std::endl;
    
    // Test sequential consistency CAS
    cas_counter.store(0);
    std::vector<std::thread> seq_cst_threads;
    
    for (int i = 0; i < 4; ++i) {
        seq_cst_threads.emplace_back([]() {
            for (int j = 0; j < 250; ++j) {
                cas_increment_seq_cst();
            }
        });
    }
    
    for (auto& t : seq_cst_threads) {
        t.join();
    }
    
    std::cout << "Sequential CAS result: " << cas_counter.load() << std::endl;
}

// =============================================================================
// 7. PERFORMANCE IMPLICATIONS
// =============================================================================

void demonstrate_performance_implications() {
    SECTION("Performance Implications");
    
    std::cout << "Memory ordering performance (fastest to slowest):" << std::endl;
    std::cout << "1. memory_order_relaxed - No synchronization overhead" << std::endl;
    std::cout << "2. memory_order_acquire/release - Moderate overhead" << std::endl;
    std::cout << "3. memory_order_seq_cst - Highest overhead" << std::endl;
    
    std::cout << "\nChoosing memory ordering:" << std::endl;
    std::cout << "- Use relaxed for counters without dependencies" << std::endl;
    std::cout << "- Use acquire-release for producer-consumer patterns" << std::endl;
    std::cout << "- Use seq_cst when total ordering is required" << std::endl;
    std::cout << "- Consider memory fences for complex synchronization" << std::endl;
    
    // Simple performance demonstration
    const int iterations = 1000000;
    std::atomic<int> perf_counter{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        perf_counter.fetch_add(1, std::memory_order_relaxed);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Relaxed operations took: " << duration.count() << " microseconds" << std::endl;
}

int main() {
    std::cout << "MEMORY MODEL AND CONSISTENCY TUTORIAL\n";
    std::cout << "====================================\n";
    
    demonstrate_memory_ordering_basics();
    demonstrate_relaxed_ordering();
    demonstrate_acquire_release();
    demonstrate_sequential_consistency();
    demonstrate_memory_fences();
    demonstrate_cas_memory_ordering();
    demonstrate_performance_implications();
    
    return 0;
}

/*
KEY CONCEPTS:
- Memory ordering controls how memory operations are synchronized
- Relaxed ordering: no synchronization, best performance
- Acquire-release: synchronizes producer-consumer patterns
- Sequential consistency: total ordering, easiest to reason about
- Memory fences provide synchronization without atomic operations
- Compare-and-swap supports different memory orderings
- Stronger ordering = more synchronization = lower performance
- Choose appropriate ordering based on synchronization needs
*/
