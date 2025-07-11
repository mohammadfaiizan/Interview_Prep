/*
 * CONCURRENCY AND MULTITHREADING - MEMORY MODEL AND CONSISTENCY
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
    std::cout << "1. memory_order_relaxed - No synchronization" << std::endl;
    std::cout << "2. memory_order_acquire - Acquire operation" << std::endl;
    std::cout << "3. memory_order_release - Release operation" << std::endl;
    std::cout << "4. memory_order_acq_rel - Both acquire and release" << std::endl;
    std::cout << "5. memory_order_seq_cst - Sequential consistency (default)" << std::endl;
    
    std::atomic<int> counter{0};
    
    // Different orderings in action
    counter.store(10, std::memory_order_relaxed);
    counter.store(20, std::memory_order_release);
    counter.store(30, std::memory_order_seq_cst);
    
    int val1 = counter.load(std::memory_order_relaxed);
    int val2 = counter.load(std::memory_order_acquire);
    int val3 = counter.load(std::memory_order_seq_cst);
    
    std::cout << "Final values: " << val1 << ", " << val2 << ", " << val3 << std::endl;
}

// =============================================================================
// 2. RELAXED ORDERING
// =============================================================================

std::atomic<int> relaxed_counter{0};
std::atomic<int> relaxed_data[3];

void relaxed_worker(int id) {
    for (int i = 0; i < 1000; ++i) {
        relaxed_data[id].store(i, std::memory_order_relaxed);
        relaxed_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void demonstrate_relaxed_ordering() {
    SECTION("Relaxed Memory Ordering");
    
    relaxed_counter.store(0);
    for (int i = 0; i < 3; ++i) {
        relaxed_data[i].store(0);
    }
    
    std::vector<std::thread> workers;
    for (int i = 0; i < 3; ++i) {
        workers.emplace_back(relaxed_worker, i);
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "Relaxed counter: " << relaxed_counter.load() << std::endl;
    std::cout << "Data values: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << relaxed_data[i].load() << " ";
    }
    std::cout << std::endl;
    std::cout << "Relaxed: No ordering guarantees between operations" << std::endl;
}

// =============================================================================
// 3. ACQUIRE-RELEASE ORDERING
// =============================================================================

std::atomic<bool> ready{false};
std::atomic<int> payload{0};

void acquire_release_producer() {
    payload.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);  // Release
}

void acquire_release_consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire
        std::this_thread::yield();
    }
    int data = payload.load(std::memory_order_relaxed);
    std::cout << "Consumer received: " << data << std::endl;
}

void demonstrate_acquire_release() {
    SECTION("Acquire-Release Ordering");
    
    ready.store(false);
    payload.store(0);
    
    std::thread producer(acquire_release_producer);
    std::thread consumer(acquire_release_consumer);
    
    producer.join();
    consumer.join();
    
    std::cout << "Acquire-release: Synchronizes producer and consumer" << std::endl;
}

// =============================================================================
// 4. SEQUENTIAL CONSISTENCY
// =============================================================================

std::atomic<bool> x{false}, y{false};
std::atomic<int> z{0};

void seq_cst_thread1() {
    x.store(true);  // Default is seq_cst
    if (y.load()) {
        z.fetch_add(1);
    }
}

void seq_cst_thread2() {
    y.store(true);  // Default is seq_cst
    if (x.load()) {
        z.fetch_add(1);
    }
}

void demonstrate_sequential_consistency() {
    SECTION("Sequential Consistency");
    
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
    std::cout << "Sequential consistency: Total ordering of all operations" << std::endl;
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
    
    std::cout << "Memory fences: Provide ordering without atomic operations" << std::endl;
}

// =============================================================================
// 6. COMPARE AND SWAP ORDERING
// =============================================================================

std::atomic<int> cas_counter{0};

void cas_worker_relaxed() {
    for (int i = 0; i < 1000; ++i) {
        int expected = cas_counter.load(std::memory_order_relaxed);
        while (!cas_counter.compare_exchange_weak(expected, expected + 1, 
                                                 std::memory_order_relaxed)) {
            // Retry
        }
    }
}

void cas_worker_seq_cst() {
    for (int i = 0; i < 1000; ++i) {
        int expected = cas_counter.load();
        while (!cas_counter.compare_exchange_weak(expected, expected + 1)) {
            // Default is seq_cst
        }
    }
}

void demonstrate_cas_ordering() {
    SECTION("Compare-and-Swap Ordering");
    
    // Test with relaxed ordering
    cas_counter.store(0);
    std::vector<std::thread> relaxed_threads;
    
    for (int i = 0; i < 4; ++i) {
        relaxed_threads.emplace_back(cas_worker_relaxed);
    }
    
    for (auto& t : relaxed_threads) {
        t.join();
    }
    
    std::cout << "Relaxed CAS result: " << cas_counter.load() << std::endl;
    
    // Test with sequential consistency
    cas_counter.store(0);
    std::vector<std::thread> seq_cst_threads;
    
    for (int i = 0; i < 4; ++i) {
        seq_cst_threads.emplace_back(cas_worker_seq_cst);
    }
    
    for (auto& t : seq_cst_threads) {
        t.join();
    }
    
    std::cout << "Sequential CAS result: " << cas_counter.load() << std::endl;
}

// =============================================================================
// 7. BEST PRACTICES
// =============================================================================

void demonstrate_best_practices() {
    SECTION("Best Practices");
    
    std::cout << "Memory Ordering Guidelines:" << std::endl;
    std::cout << "1. Use relaxed for simple counters without dependencies" << std::endl;
    std::cout << "2. Use acquire-release for producer-consumer patterns" << std::endl;
    std::cout << "3. Use seq_cst when total ordering is required" << std::endl;
    std::cout << "4. Prefer stronger ordering when in doubt" << std::endl;
    std::cout << "5. Profile to verify performance benefits of weaker ordering" << std::endl;
    
    std::cout << "\nPerformance Impact (fastest to slowest):" << std::endl;
    std::cout << "relaxed > acquire/release > seq_cst" << std::endl;
    
    std::cout << "\nCommon Patterns:" << std::endl;
    std::cout << "• Flag + data: release store, acquire load" << std::endl;
    std::cout << "• Reference counting: relaxed increment, acquire decrement" << std::endl;
    std::cout << "• Lock-free algorithms: usually require seq_cst" << std::endl;
    
    // Example: Proper reference counting
    std::atomic<int> ref_count{1};
    
    auto increment_ref = [&]() {
        ref_count.fetch_add(1, std::memory_order_relaxed);  // Safe with relaxed
    };
    
    auto decrement_ref = [&]() -> bool {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // Last reference, safe to delete
            return true;
        }
        return false;
    };
    
    increment_ref();
    bool should_delete = decrement_ref();
    std::cout << "Reference counting example, should delete: " << should_delete << std::endl;
}

int main() {
    std::cout << "MEMORY MODEL AND CONSISTENCY TUTORIAL\n";
    std::cout << "====================================\n";
    
    demonstrate_memory_ordering_basics();
    demonstrate_relaxed_ordering();
    demonstrate_acquire_release();
    demonstrate_sequential_consistency();
    demonstrate_memory_fences();
    demonstrate_cas_ordering();
    demonstrate_best_practices();
    
    return 0;
}

/*
KEY CONCEPTS:
- Memory ordering controls how operations are synchronized
- Relaxed: no synchronization, best performance
- Acquire-release: synchronizes specific operations
- Sequential consistency: total ordering, easiest to reason about
- Memory fences provide ordering without atomic operations
- CAS operations support all memory orderings
- Choose ordering based on synchronization requirements
- Stronger ordering = more synchronization = lower performance
*/
