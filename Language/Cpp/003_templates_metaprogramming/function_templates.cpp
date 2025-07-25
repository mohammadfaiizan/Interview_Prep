/*
 * FUNCTION TEMPLATES - Advanced C++ Template Programming
 * 
 * This comprehensive tutorial covers:
 * 1. Basic Function Templates
 * 2. Template Argument Deduction
 * 3. Explicit Template Specialization
 * 4. Partial Template Specialization (not for functions, but workarounds)
 * 5. SFINAE (Substitution Failure Is Not An Error)
 * 6. Template Overloading
 * 7. Variadic Templates
 * 8. Perfect Forwarding
 * 9. Template Metaprogramming with Functions
 * 10. Modern C++ Template Features
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -g -O2 function_templates.cpp -o function_templates
 * Advanced: g++ -std=c++20 -Wall -Wextra -g -O2 function_templates.cpp -o function_templates
 */

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include <chrono>
#include <memory>
#include <algorithm>
#include <functional>
#include <typeinfo>
#include <cassert>

// Debug macros for template debugging
#define DEBUG_TEMPLATE(x) std::cout << "[TEMPLATE DEBUG] " << #x << " = " << (x) << std::endl
#define TEMPLATE_INFO(msg) std::cout << "[TEMPLATE INFO] " << msg << std::endl
#define TYPE_INFO(T) std::cout << "[TYPE INFO] " << #T << " = " << typeid(T).name() << std::endl

// Performance measurement macro
#define MEASURE_TIME(code) \
    do { \
        auto start = std::chrono::high_resolution_clock::now(); \
        code; \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); \
        std::cout << "[PERFORMANCE] " << #code << " took " << duration.count() << " microseconds" << std::endl; \
    } while(0)

namespace FunctionTemplates {

// ============================================================================
// 1. BASIC FUNCTION TEMPLATES
// ============================================================================

// Simple function template
template<typename T>
T maximum(T a, T b) {
    TEMPLATE_INFO("maximum<T> called");
    TYPE_INFO(T);
    return (a > b) ? a : b;
}

// Function template with multiple parameters
template<typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    TEMPLATE_INFO("add<T,U> called");
    TYPE_INFO(T);
    TYPE_INFO(U);
    return a + b;
}

// Function template with non-type parameters
template<typename T, int N>
T multiply_by_constant(T value) {
    TEMPLATE_INFO("multiply_by_constant<T," + std::to_string(N) + "> called");
    return value * N;
}

// Function template with default template arguments
template<typename T = int, int N = 1>
T power(T base) {
    T result = base;
    for (int i = 1; i < N; ++i) {
        result *= base;
    }
    return result;
}

// ============================================================================
// 2. TEMPLATE ARGUMENT DEDUCTION
// ============================================================================

// Automatic template argument deduction
template<typename T>
void print_type_info(T&& value) {
    std::cout << "Value: " << value << std::endl;
    std::cout << "Type: " << typeid(T).name() << std::endl;
    std::cout << "Is lvalue reference: " << std::is_lvalue_reference_v<T> << std::endl;
    std::cout << "Is rvalue reference: " << std::is_rvalue_reference_v<T> << std::endl;
    std::cout << "---" << std::endl;
}

// Template argument deduction with arrays
template<typename T, size_t N>
size_t array_size(T (&)[N]) {
    return N;
}

// Template argument deduction with function pointers
template<typename R, typename... Args>
void analyze_function(R(*func)(Args...)) {
    std::cout << "Function pointer analysis:" << std::endl;
    std::cout << "Return type: " << typeid(R).name() << std::endl;
    std::cout << "Argument count: " << sizeof...(Args) << std::endl;
}

// ============================================================================
// 3. EXPLICIT TEMPLATE SPECIALIZATION
// ============================================================================

// Primary template
template<typename T>
std::string type_name() {
    return "Unknown type";
}

// Explicit specializations
template<>
std::string type_name<int>() {
    return "Integer";
}

template<>
std::string type_name<double>() {
    return "Double precision floating point";
}

template<>
std::string type_name<std::string>() {
    return "Standard string";
}

// Specialization for pointer types
template<typename T>
std::string type_name<T*>() {
    return "Pointer to " + type_name<T>();
}

// ============================================================================
// 4. SFINAE (Substitution Failure Is Not An Error)
// ============================================================================

// SFINAE with enable_if
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
safe_divide(T a, T b) {
    TEMPLATE_INFO("safe_divide for integral types");
    return (b != 0) ? a / b : 0;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
safe_divide(T a, T b) {
    TEMPLATE_INFO("safe_divide for floating point types");
    return (b != 0.0) ? a / b : std::numeric_limits<T>::quiet_NaN();
}

// SFINAE with decltype and expression validity
template<typename T>
auto has_size_method(T&& t) -> decltype(t.size(), std::true_type{}) {
    return std::true_type{};
}

template<typename T>
std::false_type has_size_method(...) {
    return std::false_type{};
}

// Modern SFINAE with if constexpr (C++17)
template<typename T>
void print_container_or_value(T&& container) {
    if constexpr (std::is_same_v<decltype(has_size_method(container)), std::true_type>) {
        std::cout << "Container with size: " << container.size() << std::endl;
        for (const auto& item : container) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Single value: " << container << std::endl;
    }
}

// ============================================================================
// 5. TEMPLATE OVERLOADING
// ============================================================================

// Overloaded templates with different constraints
template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
process_value(T value) {
    std::cout << "Processing arithmetic value: " << value << std::endl;
}

template<typename T>
typename std::enable_if<std::is_same<T, std::string>::value, void>::type
process_value(T value) {
    std::cout << "Processing string value: " << value << std::endl;
}

template<typename T>
typename std::enable_if<std::is_pointer<T>::value, void>::type
process_value(T value) {
    std::cout << "Processing pointer value: " << static_cast<void*>(value) << std::endl;
}

// ============================================================================
// 6. VARIADIC TEMPLATES
// ============================================================================

// Basic variadic template
template<typename... Args>
void print_values(Args... args) {
    std::cout << "Number of arguments: " << sizeof...(args) << std::endl;
    ((std::cout << args << " "), ...); // C++17 fold expression
    std::cout << std::endl;
}

// Recursive variadic template (C++11/14 style)
template<typename T>
T sum_recursive(T value) {
    return value;
}

template<typename T, typename... Args>
T sum_recursive(T first, Args... rest) {
    return first + sum_recursive(rest...);
}

// Fold expression variadic template (C++17)
template<typename... Args>
auto sum_fold(Args... args) {
    return (args + ...);
}

// Variadic template with perfect forwarding
template<typename F, typename... Args>
auto call_function(F&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
    TEMPLATE_INFO("call_function with perfect forwarding");
    return func(std::forward<Args>(args)...);
}

// ============================================================================
// 7. PERFECT FORWARDING
// ============================================================================

// Perfect forwarding example
template<typename T>
void receive_value(T&& value) {
    std::cout << "Received: " << value << std::endl;
    std::cout << "Value category: ";
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "lvalue reference" << std::endl;
    } else if constexpr (std::is_rvalue_reference_v<T>) {
        std::cout << "rvalue reference" << std::endl;
    } else {
        std::cout << "value" << std::endl;
    }
}

template<typename T>
void forward_value(T&& value) {
    TEMPLATE_INFO("Forwarding value");
    receive_value(std::forward<T>(value));
}

// Factory function with perfect forwarding
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_perfect(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// ============================================================================
// 8. TEMPLATE METAPROGRAMMING
// ============================================================================

// Compile-time factorial
template<int N>
struct factorial {
    static constexpr int value = N * factorial<N-1>::value;
};

template<>
struct factorial<0> {
    static constexpr int value = 1;
};

// Template metafunction to check if type is container
template<typename T>
struct is_container : std::false_type {};

template<typename T>
struct is_container<std::vector<T>> : std::true_type {};

// Type list manipulation
template<typename... Types>
struct type_list {};

template<typename List>
struct list_size;

template<typename... Types>
struct list_size<type_list<Types...>> {
    static constexpr size_t value = sizeof...(Types);
};

// ============================================================================
// 9. ADVANCED TEMPLATE TECHNIQUES
// ============================================================================

// Template template parameters
template<template<typename> class Container, typename T>
void print_container_info(const Container<T>& container) {
    std::cout << "Container size: " << container.size() << std::endl;
    std::cout << "Element type: " << typeid(T).name() << std::endl;
}

// CRTP (Curiously Recurring Template Pattern) helper
template<typename Derived>
class Printable {
public:
    void print() const {
        static_cast<const Derived*>(this)->print_impl();
    }
};

class MyClass : public Printable<MyClass> {
public:
    void print_impl() const {
        std::cout << "MyClass instance" << std::endl;
    }
};

// ============================================================================
// 10. MODERN C++ TEMPLATE FEATURES
// ============================================================================

// Concepts (C++20) - simplified version for C++17
template<typename T>
constexpr bool Numeric = std::is_arithmetic_v<T>;

template<typename T>
std::enable_if_t<Numeric<T>, T> constrained_add(T a, T b) {
    return a + b;
}

// Auto return type deduction
template<typename T, typename U>
auto modern_multiply(T a, U b) {
    return a * b;
}

// Variable templates
template<typename T>
constexpr bool is_pointer_v = std::is_pointer_v<T>;

template<typename T>
constexpr size_t type_size_v = sizeof(T);

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

void demonstrate_basic_templates() {
    std::cout << "\n=== BASIC FUNCTION TEMPLATES ===" << std::endl;
    
    DEBUG_TEMPLATE(maximum(10, 20));
    DEBUG_TEMPLATE(maximum(3.14, 2.71));
    DEBUG_TEMPLATE(maximum(std::string("hello"), std::string("world")));
    
    DEBUG_TEMPLATE(add(10, 3.14));
    DEBUG_TEMPLATE(add(std::string("Hello "), std::string("World")));
    
    DEBUG_TEMPLATE(multiply_by_constant<int, 5>(10));
    DEBUG_TEMPLATE(multiply_by_constant<double, 3>(2.5));
    
    DEBUG_TEMPLATE(power<int, 3>(2));
    DEBUG_TEMPLATE(power<double, 2>(3.14));
}

void demonstrate_template_deduction() {
    std::cout << "\n=== TEMPLATE ARGUMENT DEDUCTION ===" << std::endl;
    
    int x = 42;
    print_type_info(x);
    print_type_info(42);
    print_type_info(std::string("hello"));
    
    int arr[] = {1, 2, 3, 4, 5};
    DEBUG_TEMPLATE(array_size(arr));
    
    auto lambda = [](int a, int b) { return a + b; };
    analyze_function(static_cast<int(*)(int, int)>(lambda));
}

void demonstrate_specialization() {
    std::cout << "\n=== TEMPLATE SPECIALIZATION ===" << std::endl;
    
    std::cout << "int: " << type_name<int>() << std::endl;
    std::cout << "double: " << type_name<double>() << std::endl;
    std::cout << "string: " << type_name<std::string>() << std::endl;
    std::cout << "int*: " << type_name<int*>() << std::endl;
    std::cout << "double*: " << type_name<double*>() << std::endl;
}

void demonstrate_sfinae() {
    std::cout << "\n=== SFINAE TECHNIQUES ===" << std::endl;
    
    DEBUG_TEMPLATE(safe_divide(10, 3));
    DEBUG_TEMPLATE(safe_divide(10.0, 3.0));
    DEBUG_TEMPLATE(safe_divide(10, 0));
    DEBUG_TEMPLATE(safe_divide(10.0, 0.0));
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_container_or_value(vec);
    print_container_or_value(42);
    print_container_or_value(std::string("hello"));
}

void demonstrate_overloading() {
    std::cout << "\n=== TEMPLATE OVERLOADING ===" << std::endl;
    
    process_value(42);
    process_value(3.14);
    process_value(std::string("hello"));
    int* ptr = new int(42);
    process_value(ptr);
    delete ptr;
}

void demonstrate_variadic_templates() {
    std::cout << "\n=== VARIADIC TEMPLATES ===" << std::endl;
    
    print_values(1, 2, 3, 4, 5);
    print_values("hello", "world", 42, 3.14);
    
    DEBUG_TEMPLATE(sum_recursive(1, 2, 3, 4, 5));
    DEBUG_TEMPLATE(sum_fold(1, 2, 3, 4, 5));
    
    auto multiply = [](int a, int b) { return a * b; };
    DEBUG_TEMPLATE(call_function(multiply, 6, 7));
}

void demonstrate_perfect_forwarding() {
    std::cout << "\n=== PERFECT FORWARDING ===" << std::endl;
    
    int x = 42;
    forward_value(x);        // lvalue
    forward_value(42);       // rvalue
    forward_value(std::move(x)); // moved lvalue
    
    auto ptr = make_unique_perfect<std::string>("Hello World");
    std::cout << "Created string: " << *ptr << std::endl;
}

void demonstrate_metaprogramming() {
    std::cout << "\n=== TEMPLATE METAPROGRAMMING ===" << std::endl;
    
    std::cout << "factorial<5>: " << factorial<5>::value << std::endl;
    std::cout << "factorial<10>: " << factorial<10>::value << std::endl;
    
    std::cout << "is_container<int>: " << is_container<int>::value << std::endl;
    std::cout << "is_container<vector<int>>: " << is_container<std::vector<int>>::value << std::endl;
    
    std::cout << "type_list size: " << list_size<type_list<int, double, std::string>>::value << std::endl;
}

void demonstrate_advanced_techniques() {
    std::cout << "\n=== ADVANCED TEMPLATE TECHNIQUES ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_container_info(vec);
    
    MyClass obj;
    obj.print();
    
    std::cout << "is_pointer_v<int>: " << is_pointer_v<int> << std::endl;
    std::cout << "is_pointer_v<int*>: " << is_pointer_v<int*> << std::endl;
    std::cout << "type_size_v<int>: " << type_size_v<int> << std::endl;
    std::cout << "type_size_v<double>: " << type_size_v<double> << std::endl;
}

void demonstrate_modern_features() {
    std::cout << "\n=== MODERN C++ TEMPLATE FEATURES ===" << std::endl;
    
    DEBUG_TEMPLATE(constrained_add(10, 20));
    DEBUG_TEMPLATE(constrained_add(3.14, 2.71));
    
    DEBUG_TEMPLATE(modern_multiply(10, 3.14));
    DEBUG_TEMPLATE(modern_multiply(std::string("Hello "), 3));
}

// Performance comparison
void performance_comparison() {
    std::cout << "\n=== PERFORMANCE COMPARISON ===" << std::endl;
    
    const int iterations = 1000000;
    
    // Template version
    MEASURE_TIME({
        volatile int sum = 0;
        for (int i = 0; i < iterations; ++i) {
            sum += maximum(i, i + 1);
        }
    });
    
    // Macro version
    #define MAX_MACRO(a, b) ((a) > (b) ? (a) : (b))
    MEASURE_TIME({
        volatile int sum = 0;
        for (int i = 0; i < iterations; ++i) {
            sum += MAX_MACRO(i, i + 1);
        }
    });
    
    // Function version
    auto max_function = [](int a, int b) { return (a > b) ? a : b; };
    MEASURE_TIME({
        volatile int sum = 0;
        for (int i = 0; i < iterations; ++i) {
            sum += max_function(i, i + 1);
        }
    });
}

} // namespace FunctionTemplates

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=== FUNCTION TEMPLATES COMPREHENSIVE TUTORIAL ===" << std::endl;
    
    try {
        FunctionTemplates::demonstrate_basic_templates();
        FunctionTemplates::demonstrate_template_deduction();
        FunctionTemplates::demonstrate_specialization();
        FunctionTemplates::demonstrate_sfinae();
        FunctionTemplates::demonstrate_overloading();
        FunctionTemplates::demonstrate_variadic_templates();
        FunctionTemplates::demonstrate_perfect_forwarding();
        FunctionTemplates::demonstrate_metaprogramming();
        FunctionTemplates::demonstrate_advanced_techniques();
        FunctionTemplates::demonstrate_modern_features();
        FunctionTemplates::performance_comparison();
        
        std::cout << "\n=== TUTORIAL COMPLETED SUCCESSFULLY ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

/*
 * COMPILATION NOTES:
 * 
 * Basic compilation:
 * g++ -std=c++17 -Wall -Wextra -g -O2 function_templates.cpp -o function_templates
 * 
 * With all warnings:
 * g++ -std=c++17 -Wall -Wextra -Wpedantic -g -O2 function_templates.cpp -o function_templates
 * 
 * For C++20 features:
 * g++ -std=c++20 -Wall -Wextra -g -O2 function_templates.cpp -o function_templates
 * 
 * Debug version:
 * g++ -std=c++17 -Wall -Wextra -g -O0 -DDEBUG function_templates.cpp -o function_templates_debug
 * 
 * LEARNING OUTCOMES:
 * 
 * After studying this code, you should understand:
 * 1. How to write and use basic function templates
 * 2. Template argument deduction rules and mechanisms
 * 3. Explicit template specialization techniques
 * 4. SFINAE and template constraint programming
 * 5. Template overloading resolution
 * 6. Variadic templates and parameter packs
 * 7. Perfect forwarding and universal references
 * 8. Template metaprogramming fundamentals
 * 9. Advanced template techniques like CRTP
 * 10. Modern C++ template features and best practices
 * 
 * ADVANCED TOPICS COVERED:
 * - Template argument deduction and CTAD
 * - SFINAE and enable_if techniques
 * - Fold expressions (C++17)
 * - Perfect forwarding patterns
 * - Template metaprogramming
 * - Variable templates
 * - Template template parameters
 * - Concept-like programming (pre-C++20)
 * 
 * This tutorial provides a solid foundation for understanding
 * advanced C++ template programming and metaprogramming techniques.
 */
