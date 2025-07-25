/*
 * MODERN C++ - COROUTINES BASICS (C++20)
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra coroutines_basics.cpp -o coroutines_basics
 */

#include <iostream>
#include <coroutine>
#include <vector>
#include <string>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. SIMPLE GENERATOR COROUTINE
// =============================================================================

template<typename T>
struct Generator {
    struct promise_type {
        T current_value;
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    
    ~Generator() {
        if (handle) handle.destroy();
    }
    
    // Make it non-copyable but movable
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    Generator(Generator&& other) noexcept : handle(other.handle) {
        other.handle = {};
    }
    
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = {};
        }
        return *this;
    }
    
    bool next() {
        handle.resume();
        return !handle.done();
    }
    
    T value() const {
        return handle.promise().current_value;
    }
};

// Simple number generator
Generator<int> count_up_to(int limit) {
    std::cout << "Starting generator" << std::endl;
    for (int i = 1; i <= limit; ++i) {
        std::cout << "Yielding " << i << std::endl;
        co_yield i;
    }
    std::cout << "Generator finished" << std::endl;
}

void demonstrate_simple_generator() {
    SECTION("Simple Generator Coroutine");
    
    auto gen = count_up_to(5);
    
    std::cout << "Created generator, now consuming:" << std::endl;
    while (gen.next()) {
        std::cout << "Received: " << gen.value() << std::endl;
    }
}

// =============================================================================
// 2. FIBONACCI GENERATOR
// =============================================================================

Generator<long long> fibonacci(int count) {
    if (count <= 0) co_return;
    
    long long a = 0, b = 1;
    co_yield a;
    
    if (count == 1) co_return;
    co_yield b;
    
    for (int i = 2; i < count; ++i) {
        long long next = a + b;
        a = b;
        b = next;
        co_yield next;
    }
}

void demonstrate_fibonacci_generator() {
    SECTION("Fibonacci Generator");
    
    auto fib_gen = fibonacci(10);
    
    std::cout << "First 10 Fibonacci numbers:" << std::endl;
    while (fib_gen.next()) {
        std::cout << fib_gen.value() << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 3. SIMPLE TASK COROUTINE
// =============================================================================

struct Task {
    struct promise_type {
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        void return_void() {}
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> handle;
    
    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    
    ~Task() {
        if (handle) handle.destroy();
    }
    
    // Non-copyable, movable
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    Task(Task&& other) noexcept : handle(other.handle) {
        other.handle = {};
    }
    
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = {};
        }
        return *this;
    }
};

Task simple_task() {
    std::cout << "Task started" << std::endl;
    
    std::cout << "Before first suspension" << std::endl;
    co_await std::suspend_always{};
    std::cout << "After first suspension" << std::endl;
    
    std::cout << "Before second suspension" << std::endl;
    co_await std::suspend_always{};
    std::cout << "After second suspension" << std::endl;
    
    std::cout << "Task finished" << std::endl;
}

void demonstrate_simple_task() {
    SECTION("Simple Task Coroutine");
    
    // Note: This is a simplified example
    // Real async tasks would be more complex
    std::cout << "Creating task..." << std::endl;
    auto task = simple_task();
    std::cout << "Task created and started" << std::endl;
    
    // In a real scenario, you'd have an event loop that resumes tasks
    // This is just a demonstration
}

// =============================================================================
// 4. RANGE GENERATOR
// =============================================================================

Generator<int> range(int start, int end, int step = 1) {
    if (step > 0) {
        for (int i = start; i < end; i += step) {
            co_yield i;
        }
    } else if (step < 0) {
        for (int i = start; i > end; i += step) {
            co_yield i;
        }
    }
}

void demonstrate_range_generator() {
    SECTION("Range Generator");
    
    std::cout << "Range 0 to 10 step 2: ";
    auto range_gen = range(0, 10, 2);
    while (range_gen.next()) {
        std::cout << range_gen.value() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Range 10 to 0 step -2: ";
    auto reverse_range = range(10, 0, -2);
    while (reverse_range.next()) {
        std::cout << reverse_range.value() << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. STRING TOKENIZER GENERATOR
// =============================================================================

Generator<std::string> tokenize(const std::string& text, char delimiter) {
    std::string token;
    for (char c : text) {
        if (c == delimiter) {
            if (!token.empty()) {
                co_yield token;
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        co_yield token;
    }
}

void demonstrate_tokenizer() {
    SECTION("String Tokenizer Generator");
    
    std::string text = "hello,world,coroutines,are,cool";
    auto tokenizer = tokenize(text, ',');
    
    std::cout << "Tokens from '" << text << "':" << std::endl;
    while (tokenizer.next()) {
        std::cout << "'" << tokenizer.value() << "' ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 6. PRACTICAL EXAMPLE: LAZY EVALUATION
// =============================================================================

Generator<int> process_numbers(const std::vector<int>& numbers) {
    std::cout << "Starting number processing..." << std::endl;
    
    for (int num : numbers) {
        std::cout << "Processing " << num << std::endl;
        
        // Some expensive computation
        int result = num * num + num;
        
        std::cout << "Yielding " << result << std::endl;
        co_yield result;
    }
    
    std::cout << "Number processing complete" << std::endl;
}

void demonstrate_lazy_evaluation() {
    SECTION("Lazy Evaluation Example");
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto processor = process_numbers(numbers);
    
    std::cout << "Generator created, now consuming first 3 results:" << std::endl;
    
    for (int i = 0; i < 3 && processor.next(); ++i) {
        std::cout << "Got result: " << processor.value() << std::endl;
    }
    
    std::cout << "\nNote: Only first 3 numbers were processed due to lazy evaluation" << std::endl;
}

// =============================================================================
// 7. COROUTINE CONCEPTS AND TERMINOLOGY
// =============================================================================

void demonstrate_concepts() {
    SECTION("Coroutine Concepts and Terminology");
    
    std::cout << "Key Coroutine Concepts:" << std::endl;
    std::cout << "1. co_yield - suspends and yields a value" << std::endl;
    std::cout << "2. co_await - suspends and waits for something" << std::endl;
    std::cout << "3. co_return - returns from coroutine" << std::endl;
    std::cout << "4. promise_type - defines coroutine behavior" << std::endl;
    std::cout << "5. coroutine_handle - handle to manage coroutine" << std::endl;
    std::cout << "6. suspend_always - always suspend" << std::endl;
    std::cout << "7. suspend_never - never suspend" << std::endl;
    
    std::cout << "\nCoroutine Types:" << std::endl;
    std::cout << "- Generator: produces sequence of values" << std::endl;
    std::cout << "- Task: represents async operation" << std::endl;
    std::cout << "- Awaitable: can be co_awaited" << std::endl;
    
    std::cout << "\nBenefits:" << std::endl;
    std::cout << "- Lazy evaluation" << std::endl;
    std::cout << "- Memory efficient iteration" << std::endl;
    std::cout << "- Cleaner async code" << std::endl;
    std::cout << "- State machine generation by compiler" << std::endl;
}

int main() {
    std::cout << "COROUTINES BASICS (C++20) TUTORIAL\n";
    std::cout << "==================================\n";
    
    demonstrate_simple_generator();
    demonstrate_fibonacci_generator();
    demonstrate_simple_task();
    demonstrate_range_generator();
    demonstrate_tokenizer();
    demonstrate_lazy_evaluation();
    demonstrate_concepts();
    
    std::cout << "\nTutorial completed!\n";
    
    std::cout << "\nNote: This tutorial covers basic coroutine concepts." << std::endl;
    std::cout << "Real-world usage often involves more complex async frameworks." << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Coroutines are functions that can suspend and resume execution
- co_yield suspends and produces a value
- co_await suspends and waits for a result
- co_return returns from the coroutine
- promise_type defines the coroutine's interface and behavior
- Generators produce sequences of values lazily
- Tasks represent asynchronous operations
- Coroutines enable efficient iteration and async programming
- Compiler generates state machine automatically
- C++20 feature requiring modern compiler support
- Requires careful RAII and lifetime management
*/
