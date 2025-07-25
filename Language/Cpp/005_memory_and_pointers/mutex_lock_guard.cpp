/*
 * MEMORY AND POINTERS - MUTEX AND LOCK GUARDS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread mutex_lock_guard.cpp -o mutex_lock_guard
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC MUTEX OPERATIONS
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
    SECTION("Basic Mutex Operations");
    
    // Unsafe version
    shared_counter = 0;
    std::vector<std::thread> unsafe_threads;
    
    for (int i = 0; i < 3; ++i) {
        unsafe_threads.emplace_back(unsafe_increment, 1000);
    }
    
    for (auto& t : unsafe_threads) {
        t.join();
    }
    
    std::cout << "Unsafe result: " << shared_counter << " (expected: 3000)" << std::endl;
    
    // Safe version
    shared_counter = 0;
    std::vector<std::thread> safe_threads;
    
    for (int i = 0; i < 3; ++i) {
        safe_threads.emplace_back(safe_increment, 1000);
    }
    
    for (auto& t : safe_threads) {
        t.join();
    }
    
    std::cout << "Safe result: " << shared_counter << " (expected: 3000)" << std::endl;
}

// =============================================================================
// 2. LOCK GUARDS
// =============================================================================

std::mutex guard_mtx;
int guarded_data = 0;

void increment_with_lock_guard(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(guard_mtx);  // RAII
        guarded_data++;
        // Automatic unlock when lock goes out of scope
    }
}

void demonstrate_lock_guard() {
    SECTION("Lock Guard (RAII)");
    
    guarded_data = 0;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(increment_with_lock_guard, 500);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Lock guard result: " << guarded_data << " (expected: 2000)" << std::endl;
    std::cout << "Lock guard automatically unlocks on scope exit" << std::endl;
}

// =============================================================================
// 3. UNIQUE LOCK
// =============================================================================

std::mutex unique_mtx;

void demonstrate_unique_lock() {
    SECTION("Unique Lock");
    
    std::unique_lock<std::mutex> lock(unique_mtx);
    std::cout << "Lock acquired" << std::endl;
    
    // Can manually unlock
    lock.unlock();
    std::cout << "Lock manually released" << std::endl;
    
    // Can lock again
    lock.lock();
    std::cout << "Lock reacquired" << std::endl;
    
    // Can transfer ownership
    std::unique_lock<std::mutex> moved_lock = std::move(lock);
    std::cout << "Lock ownership transferred" << std::endl;
    
    // Can defer locking
    std::unique_lock<std::mutex> deferred_lock(unique_mtx, std::defer_lock);
    std::cout << "Deferred lock created (not locked yet)" << std::endl;
    
    deferred_lock.lock();
    std::cout << "Deferred lock now acquired" << std::endl;
    
    // Try lock
    std::unique_lock<std::mutex> try_lock(unique_mtx, std::try_to_lock);
    if (try_lock.owns_lock()) {
        std::cout << "Try lock succeeded" << std::endl;
    } else {
        std::cout << "Try lock failed (mutex already locked)" << std::endl;
    }
}

// =============================================================================
// 4. MULTIPLE MUTEX LOCKING
// =============================================================================

std::mutex mtx1, mtx2;
int resource1 = 0, resource2 = 0;

void transfer_resources(int amount, bool direction) {
    if (direction) {
        // Lock both mutexes to avoid deadlock
        std::lock(mtx1, mtx2);
        std::lock_guard<std::mutex> lock1(mtx1, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        
        resource1 -= amount;
        resource2 += amount;
        std::cout << "Transferred " << amount << " from 1 to 2" << std::endl;
    } else {
        std::lock(mtx1, mtx2);
        std::lock_guard<std::mutex> lock1(mtx1, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(mtx2, std::adopt_lock);
        
        resource2 -= amount;
        resource1 += amount;
        std::cout << "Transferred " << amount << " from 2 to 1" << std::endl;
    }
}

void demonstrate_multiple_locks() {
    SECTION("Multiple Mutex Locking");
    
    resource1 = 1000;
    resource2 = 1000;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(transfer_resources, 100, true);
        threads.emplace_back(transfer_resources, 100, false);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final resource1: " << resource1 << std::endl;
    std::cout << "Final resource2: " << resource2 << std::endl;
    std::cout << "Total: " << (resource1 + resource2) << std::endl;
}

// =============================================================================
// 5. SHARED MUTEX (READER-WRITER LOCK)
// =============================================================================

std::shared_mutex shared_mtx;
std::string shared_data = "Initial data";

void reader(int id) {
    std::shared_lock<std::shared_mutex> lock(shared_mtx);
    std::cout << "Reader " << id << " reads: " << shared_data << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void writer(int id) {
    std::unique_lock<std::shared_mutex> lock(shared_mtx);
    shared_data = "Data from writer " + std::to_string(id);
    std::cout << "Writer " << id << " wrote: " << shared_data << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void demonstrate_shared_mutex() {
    SECTION("Shared Mutex (Reader-Writer Lock)");
    
    std::vector<std::thread> threads;
    
    // Multiple readers can read simultaneously
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(reader, i);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Writer gets exclusive access
    threads.emplace_back(writer, 1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // More readers
    for (int i = 4; i <= 6; ++i) {
        threads.emplace_back(reader, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

// =============================================================================
// 6. RECURSIVE MUTEX
// =============================================================================

std::recursive_mutex recursive_mtx;
int recursive_data = 0;

void recursive_function(int depth) {
    std::lock_guard<std::recursive_mutex> lock(recursive_mtx);
    recursive_data++;
    
    std::cout << "Recursive call depth: " << depth << ", data: " << recursive_data << std::endl;
    
    if (depth > 0) {
        recursive_function(depth - 1);  // Can lock again in same thread
    }
}

void demonstrate_recursive_mutex() {
    SECTION("Recursive Mutex");
    
    recursive_data = 0;
    
    std::thread t1([]{ recursive_function(3); });
    std::thread t2([]{ recursive_function(2); });
    
    t1.join();
    t2.join();
    
    std::cout << "Final recursive data: " << recursive_data << std::endl;
}

// =============================================================================
// 7. TIMED MUTEX
// =============================================================================

std::timed_mutex timed_mtx;

void try_timed_lock(int id) {
    auto timeout = std::chrono::milliseconds(100);
    
    if (timed_mtx.try_lock_for(timeout)) {
        std::cout << "Thread " << id << " acquired timed lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        timed_mtx.unlock();
    } else {
        std::cout << "Thread " << id << " failed to acquire timed lock" << std::endl;
    }
}

void demonstrate_timed_mutex() {
    SECTION("Timed Mutex");
    
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(try_timed_lock, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    std::cout << "MUTEX AND LOCK GUARDS TUTORIAL\n";
    std::cout << "==============================\n";
    
    demonstrate_basic_mutex();
    demonstrate_lock_guard();
    demonstrate_unique_lock();
    demonstrate_multiple_locks();
    demonstrate_shared_mutex();
    demonstrate_recursive_mutex();
    demonstrate_timed_mutex();
    
    return 0;
}

/*
KEY CONCEPTS:
- Mutex provides mutual exclusion for shared resources
- lock_guard provides RAII-based locking (recommended)
- unique_lock allows manual lock/unlock and ownership transfer
- std::lock() prevents deadlock when locking multiple mutexes
- shared_mutex allows multiple readers or single writer
- recursive_mutex can be locked multiple times by same thread
- timed_mutex supports timeout operations
- Always prefer RAII lock guards over manual lock/unlock
*/
