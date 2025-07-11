/*
 * MEMORY AND POINTERS - ATOMIC OPERATIONS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread atomic_operations.cpp -o atomic_operations
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC ATOMIC OPERATIONS
// =============================================================================

void demonstrate_basic_atomics() {
    SECTION("Basic Atomic Operations");
    
    std::atomic<int> counter{0};
    std::atomic<bool> flag{false};
    
    std::cout << "Initial counter: " << counter.load() << std::endl;
    std::cout << "Is lock-free: " << counter.is_lock_free() << std::endl;
    
    // Basic operations
    counter.store(42);
    int old_val = counter.exchange(100);
    
    std::cout << "After store(42) and exchange(100):" << std::endl;
    std::cout << "Current value: " << counter.load() << std::endl;
    std::cout << "Old value from exchange: " << old_val << std::endl;
    
    // Compare and swap
    int expected = 100;
    bool success = counter.compare_exchange_strong(expected, 200);
    std::cout << "CAS success: " << success << ", current: " << counter.load() << std::endl;
    
    // Arithmetic operations
    counter.fetch_add(10);
    counter.fetch_sub(5);
    counter++;
    --counter;
    
    std::cout << "After arithmetic operations: " << counter.load() << std::endl;
}

// =============================================================================
// 2. MEMORY ORDERING
// =============================================================================

std::atomic<int> data{0};
std::atomic<bool> ready{false};

void producer() {
    data.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);  // Release semantics
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire semantics
        std::this_thread::yield();
    }
    std::cout << "Consumer read data: " << data.load(std::memory_order_relaxed) << std::endl;
}

void demonstrate_memory_ordering() {
    SECTION("Memory Ordering");
    
    std::cout << "Memory ordering types:" << std::endl;
    std::cout << "- relaxed: no synchronization" << std::endl;
    std::cout << "- acquire: synchronize with release" << std::endl;
    std::cout << "- release: synchronize with acquire" << std::endl;
    std::cout << "- seq_cst: sequential consistency (default)" << std::endl;
    
    data.store(0);
    ready.store(false);
    
    std::thread prod(producer);
    std::thread cons(consumer);
    
    prod.join();
    cons.join();
}

// =============================================================================
// 3. ATOMIC POINTER OPERATIONS
// =============================================================================

struct Node {
    int value;
    std::atomic<Node*> next;
    
    Node(int val) : value(val), next(nullptr) {}
};

class AtomicStack {
    std::atomic<Node*> head_;
    
public:
    AtomicStack() : head_(nullptr) {}
    
    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head_.load();
        
        do {
            new_node->next = old_head;
        } while (!head_.compare_exchange_weak(old_head, new_node));
    }
    
    bool pop(int& result) {
        Node* old_head = head_.load();
        
        while (old_head && !head_.compare_exchange_weak(old_head, old_head->next)) {
            // Retry if CAS failed
        }
        
        if (old_head) {
            result = old_head->value;
            delete old_head;
            return true;
        }
        return false;
    }
    
    ~AtomicStack() {
        int dummy;
        while (pop(dummy)) {}
    }
};

void demonstrate_atomic_pointers() {
    SECTION("Atomic Pointer Operations");
    
    AtomicStack stack;
    
    // Push elements
    for (int i = 1; i <= 5; ++i) {
        stack.push(i * 10);
    }
    
    // Pop elements
    int value;
    std::cout << "Popped values: ";
    while (stack.pop(value)) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 4. THREAD-SAFE COUNTER
// =============================================================================

class ThreadSafeCounter {
    std::atomic<long> count_{0};
    
public:
    void increment() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }
    
    void decrement() {
        count_.fetch_sub(1, std::memory_order_relaxed);
    }
    
    long get() const {
        return count_.load(std::memory_order_relaxed);
    }
};

void worker_increment(ThreadSafeCounter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
    }
}

void demonstrate_thread_safe_counter() {
    SECTION("Thread-Safe Counter");
    
    ThreadSafeCounter counter;
    const int num_threads = 4;
    const int iterations = 10000;
    
    std::vector<std::thread> threads;
    
    // Start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_increment, std::ref(counter), iterations);
    }
    
    // Wait for completion
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Expected: " << (num_threads * iterations) << std::endl;
    std::cout << "Actual: " << counter.get() << std::endl;
}

// =============================================================================
// 5. ATOMIC FLAGS AND SPIN LOCKS
// =============================================================================

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Spin until lock is acquired
            std::this_thread::yield();
        }
    }
    
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

SpinLock spin_lock;
int shared_data = 0;

void spin_worker(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        spin_lock.lock();
        shared_data++;
        spin_lock.unlock();
    }
}

void demonstrate_spin_lock() {
    SECTION("Spin Lock with atomic_flag");
    
    const int num_threads = 3;
    const int iterations = 1000;
    std::vector<std::thread> threads;
    
    shared_data = 0;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(spin_worker, i, iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Shared data final value: " << shared_data << std::endl;
    std::cout << "Expected: " << (num_threads * iterations) << std::endl;
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

void demonstrate_performance() {
    SECTION("Performance Considerations");
    
    std::cout << "Atomic operation characteristics:" << std::endl;
    std::cout << "- Slower than regular operations" << std::endl;
    std::cout << "- Faster than mutex for simple operations" << std::endl;
    std::cout << "- Memory ordering affects performance" << std::endl;
    std::cout << "- Lock-free algorithms can be complex" << std::endl;
    
    std::cout << "\nAtomic sizes:" << std::endl;
    std::cout << "atomic<bool>: " << sizeof(std::atomic<bool>) << " bytes" << std::endl;
    std::cout << "atomic<int>: " << sizeof(std::atomic<int>) << " bytes" << std::endl;
    std::cout << "atomic<long>: " << sizeof(std::atomic<long>) << " bytes" << std::endl;
    std::cout << "atomic<void*>: " << sizeof(std::atomic<void*>) << " bytes" << std::endl;
}

int main() {
    std::cout << "ATOMIC OPERATIONS TUTORIAL\n";
    std::cout << "==========================\n";
    
    demonstrate_basic_atomics();
    demonstrate_memory_ordering();
    demonstrate_atomic_pointers();
    demonstrate_thread_safe_counter();
    demonstrate_spin_lock();
    demonstrate_performance();
    
    return 0;
}

/*
KEY CONCEPTS:
- Atomic operations are indivisible and thread-safe
- Memory ordering controls synchronization between threads
- Compare-and-swap enables lock-free algorithms
- Atomic pointers allow lock-free data structures
- atomic_flag provides basic synchronization primitive
- Relaxed ordering for performance, stronger for correctness
- Atomic operations slower than regular but faster than mutex
- Lock-free programming is complex but can be very efficient
*/
