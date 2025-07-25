/*
 * MODERN C++ - PERFECT FORWARDING
 * Comprehensive guide to perfect forwarding (C++11+)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra perfect_forwarding.cpp -o perfect_forwarding
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <type_traits>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl
#define DEBUG_PRINT(x) std::cout << #x << " = " << (x) << std::endl

// =============================================================================
// 1. THE PROBLEM: FORWARDING WITHOUT PERFECT FORWARDING
// =============================================================================

class TestClass {
public:
    TestClass(const std::string& name) : name_(name) {
        std::cout << "TestClass copy constructor: " << name_ << std::endl;
    }
    
    TestClass(std::string&& name) : name_(std::move(name)) {
        std::cout << "TestClass move constructor: " << name_ << std::endl;
    }
    
    const std::string& getName() const { return name_; }
    
private:
    std::string name_;
};

// Traditional forwarding (problematic)
template<typename T>
void bad_forward_by_value(T arg) {
    std::cout << "bad_forward_by_value called" << std::endl;
    TestClass obj(arg);  // Always copies, even if T is moved
}

template<typename T>
void bad_forward_by_lvalue_ref(T& arg) {
    std::cout << "bad_forward_by_lvalue_ref called" << std::endl;
    TestClass obj(arg);  // Cannot accept rvalues
}

template<typename T>
void bad_forward_by_const_ref(const T& arg) {
    std::cout << "bad_forward_by_const_ref called" << std::endl;
    TestClass obj(arg);  // Always copies, loses move semantics
}

void demonstrate_forwarding_problem() {
    SECTION("The Forwarding Problem");
    
    std::string str = "test";
    
    std::cout << "\n--- bad_forward_by_value ---" << std::endl;
    bad_forward_by_value(str);                    // Copy
    bad_forward_by_value(std::string("temp"));    // Copy (should be move)
    
    std::cout << "\n--- bad_forward_by_lvalue_ref ---" << std::endl;
    bad_forward_by_lvalue_ref(str);               // OK
    // bad_forward_by_lvalue_ref(std::string("temp")); // Error: can't bind rvalue to lvalue ref
    
    std::cout << "\n--- bad_forward_by_const_ref ---" << std::endl;
    bad_forward_by_const_ref(str);                // Copy
    bad_forward_by_const_ref(std::string("temp"));// Copy (should be move)
}

// =============================================================================
// 2. UNIVERSAL REFERENCES (FORWARDING REFERENCES)
// =============================================================================

template<typename T>
void demonstrate_universal_reference(T&& param) {
    std::cout << "\nType analysis:" << std::endl;
    std::cout << "T = " << typeid(T).name() << std::endl;
    std::cout << "T&& = " << typeid(T&&).name() << std::endl;
    
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "T is lvalue reference" << std::endl;
    } else {
        std::cout << "T is not lvalue reference" << std::endl;
    }
    
    if constexpr (std::is_rvalue_reference_v<T&&>) {
        std::cout << "T&& is rvalue reference" << std::endl;
    } else {
        std::cout << "T&& is lvalue reference (collapsed)" << std::endl;
    }
}

void demonstrate_universal_references() {
    SECTION("Universal References (Forwarding References)");
    
    int x = 42;
    const int cx = 100;
    
    std::cout << "\n--- Passing lvalue ---" << std::endl;
    demonstrate_universal_reference(x);    // T = int&, T&& = int&
    
    std::cout << "\n--- Passing const lvalue ---" << std::endl;
    demonstrate_universal_reference(cx);   // T = const int&, T&& = const int&
    
    std::cout << "\n--- Passing rvalue ---" << std::endl;
    demonstrate_universal_reference(42);   // T = int, T&& = int&&
    
    std::cout << "\n--- Passing moved lvalue ---" << std::endl;
    demonstrate_universal_reference(std::move(x)); // T = int, T&& = int&&
}

// =============================================================================
// 3. REFERENCE COLLAPSING RULES
// =============================================================================

void demonstrate_reference_collapsing() {
    SECTION("Reference Collapsing Rules");
    
    std::cout << "Reference collapsing rules:" << std::endl;
    std::cout << "T&  &  -> T&   (lvalue ref + lvalue ref = lvalue ref)" << std::endl;
    std::cout << "T&  && -> T&   (lvalue ref + rvalue ref = lvalue ref)" << std::endl;
    std::cout << "T&& &  -> T&   (rvalue ref + lvalue ref = lvalue ref)" << std::endl;
    std::cout << "T&& && -> T&&  (rvalue ref + rvalue ref = rvalue ref)" << std::endl;
    
    // This is automatically handled by the compiler
    using int_lref = int&;
    using int_rref = int&&;
    
    // These would collapse according to the rules above
    static_assert(std::is_same_v<int_lref&, int&>);
    static_assert(std::is_same_v<int_rref&, int&>);
    static_assert(std::is_same_v<int_lref&&, int&>);
    static_assert(std::is_same_v<int_rref&&, int&&>);
    
    std::cout << "Reference collapsing rules verified at compile time!" << std::endl;
}

// =============================================================================
// 4. PERFECT FORWARDING WITH STD::FORWARD
// =============================================================================

template<typename T>
void perfect_forward_to_constructor(T&& arg) {
    std::cout << "perfect_forward_to_constructor called" << std::endl;
    // std::forward preserves the value category of the original argument
    TestClass obj(std::forward<T>(arg));
}

// Factory function using perfect forwarding
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_perfect(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void demonstrate_perfect_forwarding() {
    SECTION("Perfect Forwarding with std::forward");
    
    std::string str = "lvalue";
    
    std::cout << "\n--- Forwarding lvalue ---" << std::endl;
    perfect_forward_to_constructor(str);  // Preserves lvalue-ness
    
    std::cout << "\n--- Forwarding rvalue ---" << std::endl;
    perfect_forward_to_constructor(std::string("rvalue"));  // Preserves rvalue-ness
    
    std::cout << "\n--- Forwarding moved lvalue ---" << std::endl;
    perfect_forward_to_constructor(std::move(str));  // Preserves rvalue-ness
    
    std::cout << "\n--- Using perfect forwarding factory ---" << std::endl;
    auto obj1 = make_unique_perfect<TestClass>(std::string("factory_rvalue"));
    std::string factory_str = "factory_lvalue";
    auto obj2 = make_unique_perfect<TestClass>(factory_str);
}

// =============================================================================
// 5. VARIADIC PERFECT FORWARDING
// =============================================================================

class MultiParamClass {
public:
    MultiParamClass(int i, const std::string& s, double d) 
        : int_val_(i), str_val_(s), double_val_(d) {
        std::cout << "MultiParamClass constructor: " << i << ", " << s << ", " << d << std::endl;
    }
    
    MultiParamClass(int i, std::string&& s, double d) 
        : int_val_(i), str_val_(std::move(s)), double_val_(d) {
        std::cout << "MultiParamClass move constructor: " << i << ", " << str_val_ << ", " << d << std::endl;
    }
    
private:
    int int_val_;
    std::string str_val_;
    double double_val_;
};

template<typename T, typename... Args>
void create_object_perfect(Args&&... args) {
    std::cout << "Creating object with " << sizeof...(args) << " arguments" << std::endl;
    T obj(std::forward<Args>(args)...);
}

void demonstrate_variadic_perfect_forwarding() {
    SECTION("Variadic Perfect Forwarding");
    
    std::string str = "test";
    
    std::cout << "\n--- Forwarding multiple arguments (lvalue string) ---" << std::endl;
    create_object_perfect<MultiParamClass>(42, str, 3.14);
    
    std::cout << "\n--- Forwarding multiple arguments (rvalue string) ---" << std::endl;
    create_object_perfect<MultiParamClass>(100, std::string("moved"), 2.71);
}

// =============================================================================
// 6. COMMON PITFALLS AND SOLUTIONS
// =============================================================================

// Pitfall 1: Named rvalue references are lvalues
template<typename T>
void pitfall_named_rvalue_ref(T&& arg) {
    std::cout << "\nPitfall: Named rvalue references are lvalues" << std::endl;
    
    // arg is an lvalue even if T&& is rvalue reference!
    TestClass obj1(arg);  // Always calls copy constructor
    
    // Need std::forward to preserve original value category
    TestClass obj2(std::forward<T>(arg));  // Preserves value category
}

// Pitfall 2: Don't forward the same argument multiple times
template<typename T>
void pitfall_multiple_forward(T&& arg) {
    std::cout << "\nPitfall: Don't forward multiple times" << std::endl;
    
    TestClass obj1(std::forward<T>(arg));  // OK: first use
    // TestClass obj2(std::forward<T>(arg));  // Dangerous: second use of moved-from object
    
    // Instead, forward only once or use differently:
    TestClass obj2(arg);  // OK: use as lvalue for second construction
}

void demonstrate_pitfalls() {
    SECTION("Common Pitfalls and Solutions");
    
    std::string str = "pitfall_test";
    
    pitfall_named_rvalue_ref(str);
    pitfall_named_rvalue_ref(std::string("temp"));
    
    pitfall_multiple_forward(std::string("multi_forward_test"));
}

// =============================================================================
// 7. REAL-WORLD EXAMPLE: THREAD POOL WITH PERFECT FORWARDING
// =============================================================================

#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class SimpleThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
    
public:
    SimpleThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    
    // Perfect forwarding to enqueue function with arguments
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
    }
};

void worker_function(int id, const std::string& message) {
    std::cout << "Worker " << id << ": " << message << std::endl;
}

void demonstrate_real_world_example() {
    SECTION("Real-World Example: Thread Pool with Perfect Forwarding");
    
    SimpleThreadPool pool(2);
    
    std::string msg = "Hello from lvalue";
    
    // Perfect forwarding allows efficient parameter passing
    auto result1 = pool.enqueue(worker_function, 1, msg);  // lvalue forwarded
    auto result2 = pool.enqueue(worker_function, 2, std::string("Hello from rvalue"));  // rvalue forwarded
    
    result1.get();
    result2.get();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    std::cout << "PERFECT FORWARDING TUTORIAL\n";
    std::cout << "===========================\n";
    
    demonstrate_forwarding_problem();
    demonstrate_universal_references();
    demonstrate_reference_collapsing();
    demonstrate_perfect_forwarding();
    demonstrate_variadic_perfect_forwarding();
    demonstrate_pitfalls();
    demonstrate_real_world_example();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Universal references (T&&) in template context can bind to both lvalues and rvalues
- Reference collapsing: only T&& && becomes T&&, all others become T&
- std::forward<T>(arg) preserves the value category of the original argument
- Named rvalue references are lvalues - need std::forward to preserve rvalue-ness
- Perfect forwarding enables efficient parameter passing in generic code
- Variadic templates with perfect forwarding allow arbitrary argument forwarding
- Don't forward the same argument multiple times (use-after-move)
- Essential for factory functions, wrappers, and generic programming
*/
