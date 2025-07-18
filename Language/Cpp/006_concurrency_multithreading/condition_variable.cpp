/*
 * CONCURRENCY AND MULTITHREADING - CONDITION VARIABLES
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread condition_variable.cpp -o condition_variable
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC CONDITION VARIABLE USAGE
// =============================================================================

std::mutex basic_mtx;
std::condition_variable basic_cv;
bool data_ready = false;
std::string shared_data;

void data_producer() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    {
        std::lock_guard<std::mutex> lock(basic_mtx);
        shared_data = "Important data from producer";
        data_ready = true;
    }
    
    std::cout << "Producer: Data is ready, notifying consumer" << std::endl;
    basic_cv.notify_one();
}

void data_consumer() {
    std::unique_lock<std::mutex> lock(basic_mtx);
    
    std::cout << "Consumer: Waiting for data..." << std::endl;
    basic_cv.wait(lock, []{ return data_ready; });
    
    std::cout << "Consumer: Received data: " << shared_data << std::endl;
}

void demonstrate_basic_cv() {
    SECTION("Basic Condition Variable Usage");
    
    data_ready = false;
    shared_data.clear();
    
    std::thread producer(data_producer);
    std::thread consumer(data_consumer);
    
    producer.join();
    consumer.join();
}

// =============================================================================
// 2. PRODUCER-CONSUMER QUEUE
// =============================================================================

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable cv_;
    bool shutdown_ = false;
    
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!shutdown_) {
            queue_.push(item);
            cv_.notify_one();
        }
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return !queue_.empty() || shutdown_; });
        
        if (!queue_.empty()) {
            item = queue_.front();
            queue_.pop();
            return true;
        }
        return false;  // Shutdown
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
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = true;
        cv_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
};

void producer_task(ThreadSafeQueue<int>& queue, int producer_id, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        int value = producer_id * 1000 + i;
        queue.push(value);
        std::cout << "Producer " << producer_id << " produced: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void consumer_task(ThreadSafeQueue<int>& queue, int consumer_id) {
    int value;
    int count = 0;
    
    while (queue.pop(value)) {
        std::cout << "Consumer " << consumer_id << " consumed: " << value << std::endl;
        count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    std::cout << "Consumer " << consumer_id << " processed " << count << " items" << std::endl;
}

void demonstrate_producer_consumer() {
    SECTION("Producer-Consumer Queue");
    
    ThreadSafeQueue<int> queue;
    std::vector<std::thread> threads;
    
    // Start producers
    threads.emplace_back(producer_task, std::ref(queue), 1, 5);
    threads.emplace_back(producer_task, std::ref(queue), 2, 5);
    
    // Start consumers
    threads.emplace_back(consumer_task, std::ref(queue), 1);
    threads.emplace_back(consumer_task, std::ref(queue), 2);
    
    // Wait for producers to finish
    threads[0].join();
    threads[1].join();
    
    // Allow consumers to process remaining items
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Shutdown queue to stop consumers
    queue.shutdown();
    
    // Wait for consumers
    threads[2].join();
    threads[3].join();
}

// =============================================================================
// 3. TIMEOUT AND TIMED WAITS
// =============================================================================

std::mutex timeout_mtx;
std::condition_variable timeout_cv;
bool timeout_condition = false;

void timeout_setter() {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    {
        std::lock_guard<std::mutex> lock(timeout_mtx);
        timeout_condition = true;
    }
    
    timeout_cv.notify_one();
}

void demonstrate_timeouts() {
    SECTION("Timeout and Timed Waits");
    
    timeout_condition = false;
    
    std::thread setter(timeout_setter);
    
    std::unique_lock<std::mutex> lock(timeout_mtx);
    
    // Test wait_for with timeout
    std::cout << "Testing wait_for with 100ms timeout..." << std::endl;
    bool result = timeout_cv.wait_for(lock, std::chrono::milliseconds(100), 
                                     []{ return timeout_condition; });
    
    if (result) {
        std::cout << "Condition met within timeout" << std::endl;
    } else {
        std::cout << "Timeout occurred" << std::endl;
    }
    
    // Test wait_for with longer timeout
    std::cout << "Testing wait_for with 500ms timeout..." << std::endl;
    result = timeout_cv.wait_for(lock, std::chrono::milliseconds(500), 
                                []{ return timeout_condition; });
    
    if (result) {
        std::cout << "Condition met within timeout" << std::endl;
    } else {
        std::cout << "Timeout occurred" << std::endl;
    }
    
    setter.join();
}

// =============================================================================
// 4. MULTIPLE CONDITIONS AND NOTIFICATIONS
// =============================================================================

class WorkerPool {
private:
    std::mutex mtx_;
    std::condition_variable work_cv_;
    std::condition_variable complete_cv_;
    std::queue<int> tasks_;
    std::atomic<int> active_workers_{0};
    std::atomic<int> completed_tasks_{0};
    bool shutdown_ = false;
    
public:
    void add_task(int task_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!shutdown_) {
            tasks_.push(task_id);
            work_cv_.notify_one();
        }
    }
    
    void worker_thread(int worker_id) {
        while (true) {
            int task_id;
            
            {
                std::unique_lock<std::mutex> lock(mtx_);
                work_cv_.wait(lock, [this]{ return !tasks_.empty() || shutdown_; });
                
                if (shutdown_ && tasks_.empty()) {
                    break;
                }
                
                task_id = tasks_.front();
                tasks_.pop();
                active_workers_++;
            }
            
            // Simulate work
            std::cout << "Worker " << worker_id << " processing task " << task_id << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            {
                std::lock_guard<std::mutex> lock(mtx_);
                active_workers_--;
                completed_tasks_++;
            }
            
            complete_cv_.notify_all();
        }
    }
    
    void wait_for_completion(int expected_tasks) {
        std::unique_lock<std::mutex> lock(mtx_);
        complete_cv_.wait(lock, [this, expected_tasks]{ 
            return completed_tasks_.load() >= expected_tasks && active_workers_.load() == 0; 
        });
    }
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(mtx_);
        shutdown_ = true;
        work_cv_.notify_all();
    }
    
    int get_completed_tasks() const {
        return completed_tasks_.load();
    }
};

void demonstrate_multiple_conditions() {
    SECTION("Multiple Conditions and Notifications");
    
    WorkerPool pool;
    std::vector<std::thread> workers;
    
    // Start worker threads
    for (int i = 1; i <= 3; ++i) {
        workers.emplace_back(&WorkerPool::worker_thread, &pool, i);
    }
    
    // Add tasks
    for (int i = 1; i <= 10; ++i) {
        pool.add_task(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    // Wait for all tasks to complete
    std::cout << "Waiting for all tasks to complete..." << std::endl;
    pool.wait_for_completion(10);
    
    std::cout << "All tasks completed: " << pool.get_completed_tasks() << std::endl;
    
    // Shutdown workers
    pool.shutdown();
    for (auto& worker : workers) {
        worker.join();
    }
}

// =============================================================================
// 5. SPURIOUS WAKEUPS AND BEST PRACTICES
// =============================================================================

void demonstrate_spurious_wakeups() {
    SECTION("Spurious Wakeups and Best Practices");
    
    std::cout << "Best Practices for Condition Variables:" << std::endl;
    std::cout << "1. Always use predicate with wait()" << std::endl;
    std::cout << "2. Hold mutex when modifying shared state" << std::endl;
    std::cout << "3. Use notify_one() for single waiter, notify_all() for multiple" << std::endl;
    std::cout << "4. Prefer wait_for() over wait() when timeout is needed" << std::endl;
    std::cout << "5. Handle spurious wakeups with predicate functions" << std::endl;
    
    // Example of proper spurious wakeup handling
    std::mutex spurious_mtx;
    std::condition_variable spurious_cv;
    bool condition_met = false;
    
    auto safe_waiter = [&]() {
        std::unique_lock<std::mutex> lock(spurious_mtx);
        
        // CORRECT: Uses predicate to handle spurious wakeups
        spurious_cv.wait(lock, [&]{ return condition_met; });
        
        std::cout << "Condition safely met (spurious wakeups handled)" << std::endl;
    };
    
    auto condition_setter = [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(spurious_mtx);
            condition_met = true;
        }
        
        spurious_cv.notify_one();
    };
    
    std::thread waiter(safe_waiter);
    std::thread setter(condition_setter);
    
    waiter.join();
    setter.join();
}

// =============================================================================
// 6. PERFORMANCE CONSIDERATIONS
// =============================================================================

void demonstrate_performance() {
    SECTION("Performance Considerations");
    
    std::cout << "Performance Tips:" << std::endl;
    std::cout << "- Minimize critical section time" << std::endl;
    std::cout << "- Use notify_one() when only one waiter needs to wake" << std::endl;
    std::cout << "- Avoid unnecessary notify_all() calls" << std::endl;
    std::cout << "- Consider using atomic variables for simple flags" << std::endl;
    
    // Performance comparison: condition variable vs busy waiting
    const int iterations = 1000;
    std::mutex perf_mtx;
    std::condition_variable perf_cv;
    std::atomic<bool> flag{false};
    
    // Condition variable approach
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread cv_producer([&]() {
        for (int i = 0; i < iterations; ++i) {
            {
                std::lock_guard<std::mutex> lock(perf_mtx);
                flag.store(true);
            }
            perf_cv.notify_one();
            
            {
                std::unique_lock<std::mutex> lock(perf_mtx);
                perf_cv.wait(lock, [&]{ return !flag.load(); });
            }
        }
    });
    
    std::thread cv_consumer([&]() {
        for (int i = 0; i < iterations; ++i) {
            {
                std::unique_lock<std::mutex> lock(perf_mtx);
                perf_cv.wait(lock, [&]{ return flag.load(); });
                flag.store(false);
            }
            perf_cv.notify_one();
        }
    });
    
    cv_producer.join();
    cv_consumer.join();
    
    auto cv_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Condition variable time: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(cv_time).count() 
              << " microseconds" << std::endl;
}

int main() {
    std::cout << "CONDITION VARIABLES TUTORIAL\n";
    std::cout << "============================\n";
    
    demonstrate_basic_cv();
    demonstrate_producer_consumer();
    demonstrate_timeouts();
    demonstrate_multiple_conditions();
    demonstrate_spurious_wakeups();
    demonstrate_performance();
    
    return 0;
}

/*
KEY CONCEPTS:
- Condition variables enable threads to wait for conditions
- Always use with mutex and unique_lock for safety
- Predicate functions handle spurious wakeups automatically
- notify_one() wakes single thread, notify_all() wakes all
- wait_for() and wait_until() provide timeout functionality
- Producer-consumer pattern is classic use case
- Proper synchronization prevents race conditions
- Performance depends on notification strategy and critical section size
*/
