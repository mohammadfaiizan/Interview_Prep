/*
 * CONCURRENCY AND MULTITHREADING - MUTEX AND LOCK GUARDS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread mutex_lock_guard.cpp -o mutex_lock_guard
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC MUTEX USAGE
// =============================================================================

std::mutex basic_mtx;
int shared_counter = 0;

void unsafe_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        shared_counter++;  // Race condition!
    }
}

void safe_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        basic_mtx.lock();
        shared_counter++;
        basic_mtx.unlock();
    }
}

void demonstrate_basic_mutex() {
    SECTION("Basic Mutex Usage");
    
    // Unsafe version
    shared_counter = 0;
    std::vector<std::thread> unsafe_threads;
    
    for (int i = 0; i < 4; ++i) {
        unsafe_threads.emplace_back(unsafe_increment, 10000);
    }
    
    for (auto& t : unsafe_threads) {
        t.join();
    }
    
    std::cout << "Unsafe result: " << shared_counter << " (expected: 40000)" << std::endl;
    
    // Safe version
    shared_counter = 0;
    std::vector<std::thread> safe_threads;
    
    for (int i = 0; i < 4; ++i) {
        safe_threads.emplace_back(safe_increment, 10000);
    }
    
    for (auto& t : safe_threads) {
        t.join();
    }
    
    std::cout << "Safe result: " << shared_counter << " (expected: 40000)" << std::endl;
}

// =============================================================================
// 2. LOCK GUARD (RAII)
// =============================================================================

std::mutex raii_mtx;
int raii_counter = 0;

void raii_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(raii_mtx);  // RAII
        raii_counter++;
        // Automatic unlock when lock goes out of scope
    }
}

void demonstrate_lock_guard() {
    SECTION("Lock Guard (RAII)");
    
    raii_counter = 0;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(raii_increment, 10000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "RAII result: " << raii_counter << " (expected: 40000)" << std::endl;
    std::cout << "Lock guard ensures automatic unlock even with exceptions" << std::endl;
}

// =============================================================================
// 3. UNIQUE LOCK (FLEXIBLE)
// =============================================================================

std::mutex unique_mtx;
std::condition_variable cv;
bool data_ready = false;

void unique_lock_producer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::unique_lock<std::mutex> lock(unique_mtx);
    data_ready = true;
    cv.notify_one();
    // Can unlock early if needed
    lock.unlock();
    
    // Do other work without holding the lock
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void unique_lock_consumer() {
    std::unique_lock<std::mutex> lock(unique_mtx);
    cv.wait(lock, []{ return data_ready; });
    
    std::cout << "Consumer: Data is ready!" << std::endl;
    
    // Lock is still held, can continue using shared data
}

void demonstrate_unique_lock() {
    SECTION("Unique Lock (Flexible)");
    
    data_ready = false;
    
    std::thread producer(unique_lock_producer);
    std::thread consumer(unique_lock_consumer);
    
    producer.join();
    consumer.join();
    
    std::cout << "Unique lock provides flexibility for condition variables" << std::endl;
}

// =============================================================================
// 4. SHARED MUTEX (READER-WRITER)
// =============================================================================

std::shared_mutex shared_mtx;
std::vector<int> shared_data = {1, 2, 3, 4, 5};

void reader_task(int id) {
    std::shared_lock<std::shared_mutex> lock(shared_mtx);  // Shared lock
    
    std::cout << "Reader " << id << " reading: ";
    for (int value : shared_data) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void writer_task(int id) {
    std::unique_lock<std::shared_mutex> lock(shared_mtx);  // Exclusive lock
    
    std::cout << "Writer " << id << " writing..." << std::endl;
    for (int& value : shared_data) {
        value += 10;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void demonstrate_shared_mutex() {
    SECTION("Shared Mutex (Reader-Writer)");
    
    std::vector<std::thread> threads;
    
    // Multiple readers can run concurrently
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(reader_task, i);
    }
    
    // Writer excludes all other access
    threads.emplace_back(writer_task, 1);
    
    // More readers
    for (int i = 4; i <= 6; ++i) {
        threads.emplace_back(reader_task, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Shared mutex allows multiple readers, exclusive writers" << std::endl;
}

// =============================================================================
// 5. RECURSIVE MUTEX
// =============================================================================

std::recursive_mutex recursive_mtx;
int recursion_depth = 0;

void recursive_function(int depth) {
    std::lock_guard<std::recursive_mutex> lock(recursive_mtx);
    
    recursion_depth = depth;
    std::cout << "Recursion depth: " << depth << std::endl;
    
    if (depth > 0) {
        recursive_function(depth - 1);  // Same thread can lock again
    }
}

void demonstrate_recursive_mutex() {
    SECTION("Recursive Mutex");
    
    std::thread t(recursive_function, 5);
    t.join();
    
    std::cout << "Recursive mutex allows same thread to lock multiple times" << std::endl;
}

// =============================================================================
// 6. TIMED MUTEX
// =============================================================================

std::timed_mutex timed_mtx;
bool timeout_occurred = false;

void timed_worker(int id, int wait_ms) {
    auto timeout = std::chrono::milliseconds(wait_ms);
    
    if (timed_mtx.try_lock_for(timeout)) {
        std::cout << "Worker " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        timed_mtx.unlock();
    } else {
        std::cout << "Worker " << id << " timed out" << std::endl;
        timeout_occurred = true;
    }
}

void demonstrate_timed_mutex() {
    SECTION("Timed Mutex");
    
    timeout_occurred = false;
    
    std::vector<std::thread> workers;
    workers.emplace_back(timed_worker, 1, 300);  // Should succeed
    workers.emplace_back(timed_worker, 2, 100);  // Should timeout
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "Timed mutex allows timeout-based locking" << std::endl;
    std::cout << "Timeout occurred: " << (timeout_occurred ? "Yes" : "No") << std::endl;
}

// =============================================================================
// 7. PERFORMANCE COMPARISON
// =============================================================================

void demonstrate_performance() {
    SECTION("Performance Comparison");
    
    const int iterations = 100000;
    const int num_threads = 4;
    
    // Test different mutex types
    std::mutex regular_mutex;
    std::shared_mutex shared_mutex_test;
    std::recursive_mutex recursive_mutex_test;
    
    int counter = 0;
    
    // Regular mutex performance
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> regular_threads;
    for (int i = 0; i < num_threads; ++i) {
        regular_threads.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(regular_mutex);
                counter++;
            }
        });
    }
    
    for (auto& t : regular_threads) {
        t.join();
    }
    
    auto regular_time = std::chrono::high_resolution_clock::now() - start;
    
    // Shared mutex (exclusive) performance
    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> shared_threads;
    for (int i = 0; i < num_threads; ++i) {
        shared_threads.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                std::unique_lock<std::shared_mutex> lock(shared_mutex_test);
                counter++;
            }
        });
    }
    
    for (auto& t : shared_threads) {
        t.join();
    }
    
    auto shared_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance Results:" << std::endl;
    std::cout << "Regular mutex: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(regular_time).count() 
              << "ms" << std::endl;
    std::cout << "Shared mutex: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(shared_time).count() 
              << "ms" << std::endl;
    
    std::cout << "\nPerformance order (fastest to slowest):" << std::endl;
    std::cout << "mutex > shared_mutex > recursive_mutex > timed_mutex" << std::endl;
}

int main() {
    std::cout << "MUTEX AND LOCK GUARDS TUTORIAL\n";
    std::cout << "==============================\n";
    
    demonstrate_basic_mutex();
    demonstrate_lock_guard();
    demonstrate_unique_lock();
    demonstrate_shared_mutex();
    demonstrate_recursive_mutex();
    demonstrate_timed_mutex();
    demonstrate_performance();
    
    return 0;
}

/*
KEY CONCEPTS:
- Mutex provides mutual exclusion for shared resources
- Lock guard (RAII) ensures automatic unlock
- Unique lock provides flexibility for condition variables
- Shared mutex allows multiple readers, exclusive writers
- Recursive mutex allows same thread to lock multiple times
- Timed mutex supports timeout-based locking
- Always prefer RAII lock guards over manual lock/unlock
- Choose mutex type based on access patterns and requirements
*/
