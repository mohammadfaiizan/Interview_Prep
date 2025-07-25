/*
 * CONCURRENCY AND MULTITHREADING - ATOMIC OPERATIONS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread atomic_operations.cpp -o atomic_operations
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. ATOMIC FUNDAMENTALS
// =============================================================================

void demonstrate_atomic_fundamentals() {
    SECTION("Atomic Fundamentals");
    
    std::atomic<int> atomic_int{0};
    std::atomic<bool> atomic_bool{false};
    std::atomic<double> atomic_double{0.0};
    
    std::cout << "Atomic properties:" << std::endl;
    std::cout << "atomic<int> is lock-free: " << atomic_int.is_lock_free() << std::endl;
    std::cout << "atomic<bool> is lock-free: " << atomic_bool.is_lock_free() << std::endl;
    std::cout << "atomic<double> is lock-free: " << atomic_double.is_lock_free() << std::endl;
    
    // Basic operations
    atomic_int.store(42);
    int value = atomic_int.load();
    int old_value = atomic_int.exchange(100);
    
    std::cout << "Loaded value: " << value << std::endl;
    std::cout << "Exchanged old value: " << old_value << std::endl;
    std::cout << "Current value: " << atomic_int.load() << std::endl;
    
    // Compare and swap
    int expected = 100;
    bool success = atomic_int.compare_exchange_strong(expected, 200);
    std::cout << "CAS success: " << success << ", new value: " << atomic_int.load() << std::endl;
}

// =============================================================================
// 2. MEMORY ORDERING MODELS
// =============================================================================

std::atomic<int> data1{0}, data2{0};
std::atomic<bool> flag1{false}, flag2{false};

void relaxed_producer() {
    data1.store(42, std::memory_order_relaxed);
    flag1.store(true, std::memory_order_relaxed);
}

void relaxed_consumer() {
    while (!flag1.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
    std::cout << "Relaxed consumer saw: " << data1.load(std::memory_order_relaxed) << std::endl;
}

void acquire_release_producer() {
    data2.store(84, std::memory_order_relaxed);
    flag2.store(true, std::memory_order_release);  // Release
}

void acquire_release_consumer() {
    while (!flag2.load(std::memory_order_acquire)) {  // Acquire
        std::this_thread::yield();
    }
    std::cout << "Acquire-release consumer saw: " << data2.load(std::memory_order_relaxed) << std::endl;
}

void demonstrate_memory_ordering() {
    SECTION("Memory Ordering Models");
    
    std::cout << "Testing relaxed ordering:" << std::endl;
    std::thread prod1(relaxed_producer);
    std::thread cons1(relaxed_consumer);
    prod1.join();
    cons1.join();
    
    data2.store(0);
    flag2.store(false);
    
    std::cout << "Testing acquire-release ordering:" << std::endl;
    std::thread prod2(acquire_release_producer);
    std::thread cons2(acquire_release_consumer);
    prod2.join();
    cons2.join();
}

// =============================================================================
// 3. LOCK-FREE DATA STRUCTURES
// =============================================================================

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    std::atomic<Node*> head_;
    
public:
    LockFreeStack() : head_(nullptr) {}
    
    void push(const T& value) {
        Node* new_node = new Node(value);
        Node* old_head = head_.load();
        
        do {
            new_node->next = old_head;
        } while (!head_.compare_exchange_weak(old_head, new_node));
    }
    
    bool pop(T& result) {
        Node* old_head = head_.load();
        
        while (old_head && !head_.compare_exchange_weak(old_head, old_head->next)) {
            // Retry if CAS failed
        }
        
        if (old_head) {
            result = old_head->data;
            delete old_head;
            return true;
        }
        return false;
    }
    
    ~LockFreeStack() {
        T dummy;
        while (pop(dummy)) {}
    }
};

void demonstrate_lock_free_stack() {
    SECTION("Lock-Free Data Structures");
    
    LockFreeStack<int> stack;
    const int num_threads = 4;
    const int items_per_thread = 1000;
    
    // Producer threads
    std::vector<std::thread> producers;
    for (int i = 0; i < num_threads; ++i) {
        producers.emplace_back([&stack, i, items_per_thread]() {
            for (int j = 0; j < items_per_thread; ++j) {
                stack.push(i * items_per_thread + j);
            }
        });
    }
    
    // Consumer threads
    std::atomic<int> total_consumed{0};
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_threads; ++i) {
        consumers.emplace_back([&stack, &total_consumed]() {
            int value;
            int local_count = 0;
            while (total_consumed.load() < num_threads * items_per_thread) {
                if (stack.pop(value)) {
                    local_count++;
                }
                std::this_thread::yield();
            }
            total_consumed.fetch_add(local_count);
        });
    }
    
    for (auto& producer : producers) {
        producer.join();
    }
    
    // Signal consumers to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    total_consumed.store(num_threads * items_per_thread);
    
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    std::cout << "Lock-free stack operations completed" << std::endl;
}

// =============================================================================
// 4. ATOMIC COUNTERS AND STATISTICS
// =============================================================================

class AtomicStatistics {
private:
    std::atomic<long> count_{0};
    std::atomic<long> sum_{0};
    std::atomic<long> min_{LONG_MAX};
    std::atomic<long> max_{LONG_MIN};
    
public:
    void add_sample(long value) {
        count_.fetch_add(1, std::memory_order_relaxed);
        sum_.fetch_add(value, std::memory_order_relaxed);
        
        // Update min
        long current_min = min_.load(std::memory_order_relaxed);
        while (value < current_min && 
               !min_.compare_exchange_weak(current_min, value, std::memory_order_relaxed)) {
            // Retry if CAS failed
        }
        
        // Update max
        long current_max = max_.load(std::memory_order_relaxed);
        while (value > current_max && 
               !max_.compare_exchange_weak(current_max, value, std::memory_order_relaxed)) {
            // Retry if CAS failed
        }
    }
    
    void print_stats() const {
        long count = count_.load();
        long sum = sum_.load();
        long min_val = min_.load();
        long max_val = max_.load();
        
        std::cout << "Statistics:" << std::endl;
        std::cout << "Count: " << count << std::endl;
        std::cout << "Sum: " << sum << std::endl;
        std::cout << "Average: " << (count > 0 ? static_cast<double>(sum) / count : 0) << std::endl;
        std::cout << "Min: " << min_val << std::endl;
        std::cout << "Max: " << max_val << std::endl;
    }
};

void demonstrate_atomic_statistics() {
    SECTION("Atomic Counters and Statistics");
    
    AtomicStatistics stats;
    const int num_threads = 4;
    const int samples_per_thread = 10000;
    
    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&stats, i, samples_per_thread]() {
            std::mt19937 gen(i);  // Different seed per thread
            std::uniform_int_distribution<> dis(1, 1000);
            
            for (int j = 0; j < samples_per_thread; ++j) {
                stats.add_sample(dis(gen));
            }
        });
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    stats.print_stats();
}

// =============================================================================
// 5. ATOMIC FLAGS AND SPIN LOCKS
// =============================================================================

class SpinLock {
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Spin with exponential backoff
            std::this_thread::yield();
        }
    }
    
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
    
    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
};

SpinLock spin_lock;
long shared_counter = 0;

void spin_lock_worker(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        spin_lock.lock();
        shared_counter++;
        spin_lock.unlock();
    }
}

void demonstrate_spin_locks() {
    SECTION("Atomic Flags and Spin Locks");
    
    shared_counter = 0;
    const int num_threads = 4;
    const int iterations = 25000;
    
    std::vector<std::thread> workers;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(spin_lock_worker, iterations);
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Spin lock result: " << shared_counter << std::endl;
    std::cout << "Expected: " << (num_threads * iterations) << std::endl;
    std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

std::mutex regular_mutex;
std::atomic<long> atomic_counter{0};
long mutex_counter = 0;

void atomic_increment_worker(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void mutex_increment_worker(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(regular_mutex);
        mutex_counter++;
    }
}

void demonstrate_performance_comparison() {
    SECTION("Performance Comparison");
    
    const int num_threads = 4;
    const int iterations = 100000;
    
    // Test atomic operations
    atomic_counter.store(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> atomic_workers;
    for (int i = 0; i < num_threads; ++i) {
        atomic_workers.emplace_back(atomic_increment_worker, iterations);
    }
    
    for (auto& worker : atomic_workers) {
        worker.join();
    }
    
    auto atomic_time = std::chrono::high_resolution_clock::now() - start;
    
    // Test mutex operations
    mutex_counter = 0;
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> mutex_workers;
    for (int i = 0; i < num_threads; ++i) {
        mutex_workers.emplace_back(mutex_increment_worker, iterations);
    }
    
    for (auto& worker : mutex_workers) {
        worker.join();
    }
    
    auto mutex_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Atomic result: " << atomic_counter.load() << std::endl;
    std::cout << "Mutex result: " << mutex_counter << std::endl;
    std::cout << "Atomic time: " << std::chrono::duration_cast<std::chrono::microseconds>(atomic_time).count() << " μs" << std::endl;
    std::cout << "Mutex time: " << std::chrono::duration_cast<std::chrono::microseconds>(mutex_time).count() << " μs" << std::endl;
    
    double speedup = static_cast<double>(mutex_time.count()) / atomic_time.count();
    std::cout << "Atomic speedup: " << speedup << "x" << std::endl;
}

int main() {
    std::cout << "ATOMIC OPERATIONS TUTORIAL\n";
    std::cout << "==========================\n";
    
    demonstrate_atomic_fundamentals();
    demonstrate_memory_ordering();
    demonstrate_lock_free_stack();
    demonstrate_atomic_statistics();
    demonstrate_spin_locks();
    demonstrate_performance_comparison();
    
    return 0;
}

/*
KEY CONCEPTS:
- Atomic operations are indivisible and thread-safe
- Memory ordering controls synchronization guarantees
- Lock-free data structures use compare-and-swap
- Atomic flags enable efficient spin locks
- Performance varies: atomic > spin lock > mutex
- Relaxed ordering for performance, stronger for correctness
- Compare-and-swap enables lock-free algorithms
- Atomic operations form building blocks for concurrent programming
*/
