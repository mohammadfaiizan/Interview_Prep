/*
 * MODERN C++ - LAMBDA EXPRESSIONS
 * Comprehensive guide to lambda expressions (C++11/14/17/20)
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra lambda_expressions.cpp -o lambda_expressions
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <numeric>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl
#define DEBUG_PRINT(x) std::cout << #x << " = " << (x) << std::endl

// =============================================================================
// 1. BASIC LAMBDA SYNTAX
// =============================================================================

void demonstrate_basic_lambdas() {
    SECTION("Basic Lambda Syntax");
    
    // Basic lambda
    auto simple = []() { std::cout << "Hello Lambda!" << std::endl; };
    simple();
    
    // Lambda with parameters and return
    auto add = [](int a, int b) -> int { return a + b; };
    DEBUG_PRINT(add(5, 3));
    
    // Immediately invoked lambda
    auto result = [](int x) { return x * x; }(5);
    DEBUG_PRINT(result);
    
    // With STL algorithms
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto even_count = std::count_if(numbers.begin(), numbers.end(), 
                                   [](int n) { return n % 2 == 0; });
    DEBUG_PRINT(even_count);
}

// =============================================================================
// 2. CAPTURE MODES
// =============================================================================

void demonstrate_captures() {
    SECTION("Lambda Capture Modes");
    
    int x = 10, y = 20;
    
    // Capture by value
    auto by_value = [x, y]() {
        std::cout << "By value: x=" << x << ", y=" << y << std::endl;
    };
    by_value();
    
    // Capture by reference
    auto by_reference = [&x, &y]() {
        x += 5;
        y += 10;
        std::cout << "By reference: x=" << x << ", y=" << y << std::endl;
    };
    by_reference();
    DEBUG_PRINT(x); // Modified
    
    // Capture all by value [=]
    int a = 1, b = 2;
    auto all_value = [=]() {
        std::cout << "All by value: a=" << a << ", b=" << b << std::endl;
    };
    all_value();
    
    // Capture all by reference [&]
    auto all_reference = [&]() {
        a += 10;
        b += 20;
    };
    all_reference();
    DEBUG_PRINT(a);
    
    // Init capture (C++14)
    auto init_capture = [counter = 0]() mutable {
        return ++counter;
    };
    DEBUG_PRINT(init_capture()); // 1
    DEBUG_PRINT(init_capture()); // 2
    
    // Move capture
    auto unique_ptr = std::make_unique<int>(42);
    auto move_capture = [ptr = std::move(unique_ptr)]() {
        return *ptr;
    };
    DEBUG_PRINT(move_capture());
}

// =============================================================================
// 3. GENERIC LAMBDAS (C++14+)
// =============================================================================

void demonstrate_generic_lambdas() {
    SECTION("Generic Lambdas");
    
    // Auto parameters
    auto generic_add = [](auto a, auto b) {
        return a + b;
    };
    
    DEBUG_PRINT(generic_add(5, 3));
    DEBUG_PRINT(generic_add(2.5, 1.7));
    
    // Template lambda (C++20)
    #if __cplusplus >= 202002L
    auto template_lambda = []<typename T>(T value) {
        if constexpr (std::is_arithmetic_v<T>) {
            return value * 2;
        } else {
            return value;
        }
    };
    
    DEBUG_PRINT(template_lambda(21));
    #endif
    
    // Variadic lambda
    auto variadic_print = [](auto&&... args) {
        ((std::cout << args << " "), ...);
        std::cout << std::endl;
    };
    
    variadic_print(1, 2.5, "hello", 'c');
}

// =============================================================================
// 4. ADVANCED FEATURES
// =============================================================================

class LambdaContainer {
private:
    std::function<int(int)> operation;
    
public:
    LambdaContainer(std::function<int(int)> op) : operation(std::move(op)) {}
    int apply(int value) const { return operation(value); }
};

void demonstrate_advanced_features() {
    SECTION("Advanced Lambda Features");
    
    // Lambda as class member
    auto doubler = [](int x) { return x * 2; };
    LambdaContainer container(doubler);
    DEBUG_PRINT(container.apply(15));
    
    // Lambda factory
    auto create_multiplier = [](int factor) {
        return [factor](int value) { return value * factor; };
    };
    
    auto times_3 = create_multiplier(3);
    DEBUG_PRINT(times_3(7));
    
    // Recursive lambda
    std::function<int(int)> factorial = [&factorial](int n) -> int {
        return (n <= 1) ? 1 : n * factorial(n - 1);
    };
    DEBUG_PRINT(factorial(5));
    
    // Stateless lambda to function pointer
    int (*func_ptr)(int) = [](int x) { return x * 2; };
    DEBUG_PRINT(func_ptr(21));
    
    // Exception specification
    auto no_throw = [](int x) noexcept -> int { return x * x; };
    DEBUG_PRINT(no_throw(7));
}

// =============================================================================
// 5. REAL-WORLD EXAMPLES
// =============================================================================

void demonstrate_real_world_examples() {
    SECTION("Real-World Examples");
    
    // Custom sorting
    std::vector<std::string> words = {"apple", "Banana", "cherry", "Date"};
    std::sort(words.begin(), words.end(), 
              [](const std::string& a, const std::string& b) {
                  return std::tolower(a[0]) < std::tolower(b[0]);
              });
    
    std::cout << "Sorted words: ";
    for (const auto& word : words) std::cout << word << " ";
    std::cout << std::endl;
    
    // Functional programming style
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter even numbers and square them
    std::vector<int> result;
    std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(result),
                [](int n) { return n % 2 == 0; });
    
    std::transform(result.begin(), result.end(), result.begin(),
                  [](int n) { return n * n; });
    
    std::cout << "Even squares: ";
    for (int n : result) std::cout << n << " ";
    std::cout << std::endl;
    
    // Custom deleter for smart pointer
    auto custom_deleter = [](int* ptr) {
        std::cout << "Custom deleting: " << *ptr << std::endl;
        delete ptr;
    };
    
    {
        std::unique_ptr<int, decltype(custom_deleter)> ptr(new int(42), custom_deleter);
    } // Custom deleter called here
}

int main() {
    std::cout << "MODERN C++ LAMBDA EXPRESSIONS TUTORIAL\n";
    std::cout << "=====================================\n";
    
    demonstrate_basic_lambdas();
    demonstrate_captures();
    demonstrate_generic_lambdas();
    demonstrate_advanced_features();
    demonstrate_real_world_examples();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Lambda syntax: [capture](params) -> return_type { body }
- Capture modes: [=] by value, [&] by reference, mixed captures
- Generic lambdas with auto parameters (C++14)
- Template lambdas (C++20)
- Stateless lambdas convert to function pointers
- Each lambda has unique type
- Excellent for STL algorithms and functional programming
*/
