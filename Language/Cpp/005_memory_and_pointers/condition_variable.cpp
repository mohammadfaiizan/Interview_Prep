/*
 * MEMORY AND POINTERS - CONDITION VARIABLES
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread condition_variable.cpp -o condition_variable
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC CONDITION VARIABLE
// =============================================================================

std::mutex mtx;
std::condition_variable cv;
bool ready = false;
std::string data;

void worker_thread() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });  // Wait until ready is true
    std::cout << "Worker processed: " << data << std::endl;
}

void signal_thread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        data = "Hello from signal thread!";
        ready = true;
    }
    cv.notify_one();
}

void demonstrate_basic_cv() {
    SECTION("Basic Condition Variable");
    
    ready = false;
    data.clear();
    
    std::thread worker(worker_thread);
    std::thread signaler(signal_thread);
    
    worker.join();
    signaler.join();
}

// =============================================================================
// 2. PRODUCER-CONSUMER PATTERN
// =============================================================================

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable cv_;
    
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        cv_.notify_one();
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return false;
        }
        item = queue_.front();
        queue_.pop();
        return true;
    }
    
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return !queue_.empty(); });
        item = queue_.front();
        queue_.pop();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
};

void producer(ThreadSafeQueue<int>& queue, int id) {
    for (int i = 0; i < 5; ++i) {
        int value = id * 100 + i;
        queue.push(value);
        std::cout << "Producer " << id << " pushed: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void consumer(ThreadSafeQueue<int>& queue, int id) {
    int value;
    for (int i = 0; i < 5; ++i) {
        queue.wait_and_pop(value);
        std::cout << "Consumer " << id << " popped: " << value << std::endl;
    }
}

void demonstrate_producer_consumer() {
    SECTION("Producer-Consumer Pattern");
    
    ThreadSafeQueue<int> queue;
    
    std::vector<std::thread> threads;
    
    // Start producers
    threads.emplace_back(producer, std::ref(queue), 1);
    threads.emplace_back(producer, std::ref(queue), 2);
    
    // Start consumers
    threads.emplace_back(consumer, std::ref(queue), 1);
    threads.emplace_back(consumer, std::ref(queue), 2);
    
    for (auto& t : threads) {
        t.join();
    }
}

// =============================================================================
// 3. TIMEOUT OPERATIONS
// =============================================================================

std::mutex timeout_mtx;
std::condition_variable timeout_cv;
bool timeout_ready = false;

void demonstrate_timeout() {
    SECTION("Timeout Operations");
    
    timeout_ready = false;
    
    auto start = std::chrono::steady_clock::now();
    
    std::unique_lock<std::mutex> lock(timeout_mtx);
    
    // Wait with timeout
    bool result = timeout_cv.wait_for(lock, std::chrono::milliseconds(200), 
                                     []{ return timeout_ready; });
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (result) {
        std::cout << "Condition met within timeout" << std::endl;
    } else {
        std::cout << "Timeout occurred after " << duration.count() << "ms" << std::endl;
    }
    
    // Wait until specific time
    auto future_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    bool time_result = timeout_cv.wait_until(lock, future_time, []{ return timeout_ready; });
    
    std::cout << "Wait until result: " << (time_result ? "success" : "timeout") << std::endl;
}

// =============================================================================
// 4. NOTIFICATION STRATEGIES
// =============================================================================

std::mutex notify_mtx;
std::condition_variable notify_cv;
int notify_counter = 0;

void waiting_worker(int id) {
    std::unique_lock<std::mutex> lock(notify_mtx);
    notify_cv.wait(lock, [id]{ return notify_counter >= id; });
    std::cout << "Worker " << id << " activated when counter = " << notify_counter << std::endl;
}

void demonstrate_notifications() {
    SECTION("Notification Strategies");
    
    notify_counter = 0;
    
    std::vector<std::thread> workers;
    
    // Start multiple waiting workers
    for (int i = 1; i <= 5; ++i) {
        workers.emplace_back(waiting_worker, i);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Notify workers one by one
    for (int i = 1; i <= 3; ++i) {
        {
            std::lock_guard<std::mutex> lock(notify_mtx);
            notify_counter = i;
        }
        notify_cv.notify_one();  // Wake up one thread
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Notify all remaining workers
    {
        std::lock_guard<std::mutex> lock(notify_mtx);
        notify_counter = 10;  // Wake up all remaining
    }
    notify_cv.notify_all();  // Wake up all threads
    
    for (auto& worker : workers) {
        worker.join();
    }
}

// =============================================================================
// 5. SPURIOUS WAKEUPS
// =============================================================================

void demonstrate_spurious_wakeups() {
    SECTION("Spurious Wakeups");
    
    std::cout << "Spurious wakeups can occur even without notify()" << std::endl;
    std::cout << "Always use predicate in wait() to handle this:" << std::endl;
    std::cout << "  cv.wait(lock, []{ return condition; });" << std::endl;
    std::cout << "Instead of:" << std::endl;
    std::cout << "  while (!condition) cv.wait(lock);" << std::endl;
    
    // Example of proper handling
    bool condition = false;
    std::mutex spurious_mtx;
    std::condition_variable spurious_cv;
    
    auto safe_wait = [&]() {
        std::unique_lock<std::mutex> lock(spurious_mtx);
        spurious_cv.wait(lock, [&]{ return condition; });  // Predicate handles spurious wakeups
        std::cout << "Safe wait completed" << std::endl;
    };
    
    std::thread waiter(safe_wait);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        std::lock_guard<std::mutex> lock(spurious_mtx);
        condition = true;
    }
    spurious_cv.notify_one();
    
    waiter.join();
}

int main() {
    std::cout << "CONDITION VARIABLES TUTORIAL\n";
    std::cout << "============================\n";
    
    demonstrate_basic_cv();
    demonstrate_producer_consumer();
    demonstrate_timeout();
    demonstrate_notifications();
    demonstrate_spurious_wakeups();
    
    return 0;
}

/*
KEY CONCEPTS:
- Condition variables enable threads to wait for conditions
- Always use with mutex and unique_lock
- Use predicates to handle spurious wakeups
- notify_one() wakes one thread, notify_all() wakes all
- wait_for() and wait_until() provide timeout functionality
- Producer-consumer is classic use case
- Proper synchronization prevents race conditions
- Thread-safe data structures often use condition variables
*/
