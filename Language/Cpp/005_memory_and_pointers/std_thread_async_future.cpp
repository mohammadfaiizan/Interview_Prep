/*
 * MEMORY AND POINTERS - STD::THREAD, ASYNC, FUTURE
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread std_thread_async_future.cpp -o thread_async
 */

#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC THREAD OPERATIONS
// =============================================================================

void simple_task(int id) {
    std::cout << "Task " << id << " running on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Task " << id << " completed" << std::endl;
}

void demonstrate_basic_threads() {
    SECTION("Basic Thread Operations");
    
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    
    // Create and start threads
    std::thread t1(simple_task, 1);
    std::thread t2(simple_task, 2);
    
    // Wait for completion
    t1.join();
    t2.join();
    
    // Thread with lambda
    std::thread t3([](int value) {
        std::cout << "Lambda thread with value: " << value << std::endl;
    }, 42);
    
    t3.join();
    
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl;
}

// =============================================================================
// 2. ASYNC AND FUTURE
// =============================================================================

int compute_factorial(int n) {
    std::cout << "Computing factorial of " << n << std::endl;
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return result;
}

void demonstrate_async_future() {
    SECTION("Async and Future");
    
    // Launch async task
    std::future<int> future_result = std::async(std::launch::async, compute_factorial, 5);
    
    // Do other work while computation runs
    std::cout << "Doing other work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Get result (blocks until ready)
    int result = future_result.get();
    std::cout << "Factorial result: " << result << std::endl;
    
    // Multiple async tasks
    auto f1 = std::async(std::launch::async, compute_factorial, 4);
    auto f2 = std::async(std::launch::async, compute_factorial, 6);
    
    std::cout << "Results: " << f1.get() << " and " << f2.get() << std::endl;
}

// =============================================================================
// 3. PROMISE AND FUTURE
// =============================================================================

void producer_task(std::promise<std::string>&& promise) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    promise.set_value("Data from producer");
}

void demonstrate_promise_future() {
    SECTION("Promise and Future");
    
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();
    
    // Start producer thread
    std::thread producer(producer_task, std::move(promise));
    
    // Wait for result
    std::cout << "Waiting for producer..." << std::endl;
    std::string result = future.get();
    std::cout << "Received: " << result << std::endl;
    
    producer.join();
}

// =============================================================================
// 4. SHARED FUTURE
// =============================================================================

void consumer_task(std::shared_future<int> shared_future, int id) {
    int value = shared_future.get();
    std::cout << "Consumer " << id << " got value: " << value << std::endl;
}

void demonstrate_shared_future() {
    SECTION("Shared Future");
    
    std::promise<int> promise;
    std::shared_future<int> shared_future = promise.get_future().share();
    
    // Multiple consumers
    std::vector<std::thread> consumers;
    for (int i = 1; i <= 3; ++i) {
        consumers.emplace_back(consumer_task, shared_future, i);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    promise.set_value(42);
    
    for (auto& consumer : consumers) {
        consumer.join();
    }
}

// =============================================================================
// 5. FUTURE STATUS AND TIMEOUT
// =============================================================================

void demonstrate_future_status() {
    SECTION("Future Status and Timeout");
    
    auto slow_task = []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 100;
    };
    
    auto future = std::async(std::launch::async, slow_task);
    
    // Check status with timeout
    auto status = future.wait_for(std::chrono::milliseconds(100));
    
    switch (status) {
        case std::future_status::ready:
            std::cout << "Task completed: " << future.get() << std::endl;
            break;
        case std::future_status::timeout:
            std::cout << "Task still running..." << std::endl;
            break;
        case std::future_status::deferred:
            std::cout << "Task deferred" << std::endl;
            break;
    }
    
    // Wait for completion
    std::cout << "Final result: " << future.get() << std::endl;
}

// =============================================================================
// 6. PACKAGED TASK
// =============================================================================

int multiply(int a, int b) {
    return a * b;
}

void demonstrate_packaged_task() {
    SECTION("Packaged Task");
    
    // Create packaged task
    std::packaged_task<int(int, int)> task(multiply);
    std::future<int> future = task.get_future();
    
    // Run task in thread
    std::thread worker(std::move(task), 6, 7);
    
    // Get result
    int result = future.get();
    std::cout << "Packaged task result: " << result << std::endl;
    
    worker.join();
    
    // Lambda packaged task
    std::packaged_task<int()> lambda_task([]() {
        return 2 * 2 * 2;
    });
    
    auto lambda_future = lambda_task.get_future();
    std::thread lambda_worker(std::move(lambda_task));
    
    std::cout << "Lambda task result: " << lambda_future.get() << std::endl;
    lambda_worker.join();
}

// =============================================================================
// 7. EXCEPTION HANDLING
// =============================================================================

void throwing_task() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    throw std::runtime_error("Something went wrong!");
}

void demonstrate_exception_handling() {
    SECTION("Exception Handling");
    
    // Exception in async task
    auto future = std::async(std::launch::async, throwing_task);
    
    try {
        future.get();  // Exception is rethrown here
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    // Exception with promise
    std::promise<int> promise;
    auto promise_future = promise.get_future();
    
    std::thread exception_thread([&promise]() {
        try {
            throw std::runtime_error("Promise exception");
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    });
    
    try {
        promise_future.get();
    } catch (const std::exception& e) {
        std::cout << "Promise exception: " << e.what() << std::endl;
    }
    
    exception_thread.join();
}

// =============================================================================
// 8. THREAD POOL SIMULATION
// =============================================================================

class SimpleThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
public:
    SimpleThreadPool(size_t threads) : stop_(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        
                        if (stop_ && tasks_.empty()) return;
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
    
    template<class F>
    auto enqueue(F&& f) -> std::future<typename std::result_of<F()>::type> {
        using return_type = typename std::result_of<F()>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<F>(f)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }
};

void demonstrate_thread_pool() {
    SECTION("Simple Thread Pool");
    
    SimpleThreadPool pool(2);
    std::vector<std::future<int>> results;
    
    // Submit tasks
    for (int i = 0; i < 5; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return i * i;
            })
        );
    }
    
    // Collect results
    for (auto& result : results) {
        std::cout << "Pool result: " << result.get() << std::endl;
    }
}

int main() {
    std::cout << "STD::THREAD, ASYNC, FUTURE TUTORIAL\n";
    std::cout << "===================================\n";
    
    demonstrate_basic_threads();
    demonstrate_async_future();
    demonstrate_promise_future();
    demonstrate_shared_future();
    demonstrate_future_status();
    demonstrate_packaged_task();
    demonstrate_exception_handling();
    // demonstrate_thread_pool();  // Commented out due to complexity
    
    return 0;
}

/*
KEY CONCEPTS:
- std::thread creates and manages threads
- std::async runs tasks asynchronously, returns future
- std::future represents eventual result of async operation
- std::promise allows setting value from different thread
- std::shared_future allows multiple threads to wait for same result
- std::packaged_task wraps callable for async execution
- Exceptions propagate through futures
- Thread pools reuse threads for multiple tasks
*/
