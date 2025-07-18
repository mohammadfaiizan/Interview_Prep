/*
 * CONCURRENCY AND MULTITHREADING - STD::THREAD, ASYNC, AND FUTURE
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread std_thread_async_future.cpp -o thread_async_future
 */

#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <numeric>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC STD::THREAD USAGE
// =============================================================================

void simple_task(int id, int duration_ms) {
    std::cout << "Thread " << id << " starting" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    std::cout << "Thread " << id << " finished" << std::endl;
}

void demonstrate_basic_threads() {
    SECTION("Basic std::thread Usage");
    
    // Create and start threads
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(simple_task, i, i * 100);
    }
    
    std::cout << "Main thread continues while others work..." << std::endl;
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All threads completed" << std::endl;
}

// =============================================================================
// 2. THREAD WITH RETURN VALUES (PROBLEMATIC)
// =============================================================================

void problematic_return_value() {
    SECTION("Thread with Return Values (Problematic)");
    
    std::cout << "Problem: std::thread cannot directly return values" << std::endl;
    
    // This won't work - threads don't return values
    // int result = std::thread([]() { return 42; });  // ERROR!
    
    // Workaround: Use shared variable (requires synchronization)
    int shared_result = 0;
    std::mutex result_mutex;
    
    std::thread worker([&]() {
        int computation = 0;
        for (int i = 1; i <= 1000; ++i) {
            computation += i;
        }
        
        std::lock_guard<std::mutex> lock(result_mutex);
        shared_result = computation;
    });
    
    worker.join();
    
    std::cout << "Shared result: " << shared_result << std::endl;
    std::cout << "This approach requires manual synchronization" << std::endl;
}

// =============================================================================
// 3. STD::ASYNC AND STD::FUTURE
// =============================================================================

int expensive_computation(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::accumulate(std::make_unique<int[]>(n).get(), 
                          std::make_unique<int[]>(n).get() + n, 0,
                          [](int sum, int) { return sum + 1; });
}

void demonstrate_async_future() {
    SECTION("std::async and std::future");
    
    std::cout << "Starting asynchronous computations..." << std::endl;
    
    // Launch async tasks
    auto future1 = std::async(std::launch::async, expensive_computation, 1000);
    auto future2 = std::async(std::launch::async, expensive_computation, 2000);
    auto future3 = std::async(std::launch::async, expensive_computation, 3000);
    
    std::cout << "Main thread can do other work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Get results (blocks until ready)
    int result1 = future1.get();
    int result2 = future2.get();
    int result3 = future3.get();
    
    std::cout << "Results: " << result1 << ", " << result2 << ", " << result3 << std::endl;
}

// =============================================================================
// 4. LAUNCH POLICIES
// =============================================================================

void demonstrate_launch_policies() {
    SECTION("Launch Policies");
    
    auto task = []() {
        std::cout << "Task running on thread: " << std::this_thread::get_id() << std::endl;
        return 42;
    };
    
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    
    // Async launch (guaranteed new thread)
    std::cout << "\nAsync launch:" << std::endl;
    auto async_future = std::async(std::launch::async, task);
    async_future.wait();
    
    // Deferred launch (executed on get/wait)
    std::cout << "\nDeferred launch:" << std::endl;
    auto deferred_future = std::async(std::launch::deferred, task);
    deferred_future.wait();
    
    // Default (implementation chooses)
    std::cout << "\nDefault launch:" << std::endl;
    auto default_future = std::async(task);
    default_future.wait();
}

// =============================================================================
// 5. FUTURE STATUS AND TIMEOUTS
// =============================================================================

void demonstrate_future_status() {
    SECTION("Future Status and Timeouts");
    
    auto slow_task = []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return 100;
    };
    
    auto future = std::async(std::launch::async, slow_task);
    
    // Check status without blocking
    auto status = future.wait_for(std::chrono::milliseconds(50));
    
    switch (status) {
        case std::future_status::ready:
            std::cout << "Task completed: " << future.get() << std::endl;
            break;
        case std::future_status::timeout:
            std::cout << "Task still running, waiting..." << std::endl;
            std::cout << "Final result: " << future.get() << std::endl;
            break;
        case std::future_status::deferred:
            std::cout << "Task was deferred" << std::endl;
            break;
    }
}

// =============================================================================
// 6. PROMISE AND FUTURE COMMUNICATION
// =============================================================================

void demonstrate_promise_future() {
    SECTION("Promise and Future Communication");
    
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    // Producer thread
    std::thread producer([&promise]() {
        std::cout << "Producer: Computing value..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        int result = 42 * 42;
        promise.set_value(result);
        std::cout << "Producer: Value sent" << std::endl;
    });
    
    // Consumer (main thread)
    std::cout << "Consumer: Waiting for value..." << std::endl;
    int value = future.get();
    std::cout << "Consumer: Received value: " << value << std::endl;
    
    producer.join();
}

// =============================================================================
// 7. EXCEPTION HANDLING
// =============================================================================

void demonstrate_exception_handling() {
    SECTION("Exception Handling");
    
    auto risky_task = [](bool should_throw) {
        if (should_throw) {
            throw std::runtime_error("Something went wrong!");
        }
        return 42;
    };
    
    // Success case
    try {
        auto future1 = std::async(std::launch::async, risky_task, false);
        int result = future1.get();
        std::cout << "Success result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    // Exception case
    try {
        auto future2 = std::async(std::launch::async, risky_task, true);
        int result = future2.get();  // Exception will be thrown here
        std::cout << "This won't be printed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
}

// =============================================================================
// 8. PERFORMANCE COMPARISON
// =============================================================================

void cpu_intensive_task(int iterations) {
    volatile long sum = 0;
    for (int i = 0; i < iterations; ++i) {
        sum += i * i;
    }
}

void demonstrate_performance() {
    SECTION("Performance Comparison");
    
    const int num_tasks = 4;
    const int iterations = 1000000;
    
    // Sequential execution
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_tasks; ++i) {
        cpu_intensive_task(iterations);
    }
    
    auto sequential_time = std::chrono::high_resolution_clock::now() - start;
    
    // Thread-based parallel execution
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_tasks; ++i) {
        threads.emplace_back(cpu_intensive_task, iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto thread_time = std::chrono::high_resolution_clock::now() - start;
    
    // Async-based parallel execution
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < num_tasks; ++i) {
        futures.push_back(std::async(std::launch::async, cpu_intensive_task, iterations));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    auto async_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance Results:" << std::endl;
    std::cout << "Sequential: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(sequential_time).count() 
              << " ms" << std::endl;
    std::cout << "std::thread: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(thread_time).count() 
              << " ms" << std::endl;
    std::cout << "std::async: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(async_time).count() 
              << " ms" << std::endl;
    
    double thread_speedup = static_cast<double>(sequential_time.count()) / thread_time.count();
    double async_speedup = static_cast<double>(sequential_time.count()) / async_time.count();
    
    std::cout << "Thread speedup: " << thread_speedup << "x" << std::endl;
    std::cout << "Async speedup: " << async_speedup << "x" << std::endl;
}

int main() {
    std::cout << "STD::THREAD, ASYNC, AND FUTURE TUTORIAL\n";
    std::cout << "=======================================\n";
    
    demonstrate_basic_threads();
    problematic_return_value();
    demonstrate_async_future();
    demonstrate_launch_policies();
    demonstrate_future_status();
    demonstrate_promise_future();
    demonstrate_exception_handling();
    demonstrate_performance();
    
    std::cout << "\nHardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- std::thread creates and manages threads
- std::async provides higher-level task-based parallelism
- std::future enables getting results from async operations
- std::promise allows setting values from other threads
- Launch policies control execution timing (async vs deferred)
- Futures handle exceptions from async operations
- std::async is often easier to use than raw threads
- Choose based on whether you need return values and exception handling
*/
