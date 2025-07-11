/*
 * STL DEEP DIVE - FUNCTORS AND PREDICATES
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra functors_predicates.cpp -o functors_predicates
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. FUNCTION OBJECTS (FUNCTORS)
// =============================================================================

class Multiplier {
private:
    int factor_;
    
public:
    explicit Multiplier(int factor) : factor_(factor) {}
    
    int operator()(int x) const {
        return x * factor_;
    }
};

class Counter {
private:
    mutable int count_ = 0;
    
public:
    int operator()() const {
        return ++count_;
    }
    
    int get_count() const { return count_; }
};

void demonstrate_functors() {
    SECTION("Function Objects (Functors)");
    
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    // Using custom functor
    Multiplier multiply_by_3(3);
    std::transform(data.begin(), data.end(), data.begin(), multiply_by_3);
    
    std::cout << "After multiplying by 3: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Stateful functor
    Counter counter;
    std::vector<int> sequence(5);
    std::generate(sequence.begin(), sequence.end(), counter);
    
    std::cout << "Generated sequence: ";
    for (int x : sequence) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "Final count: " << counter.get_count() << std::endl;
}

// =============================================================================
// 2. PREDICATES
// =============================================================================

class IsEven {
public:
    bool operator()(int x) const {
        return x % 2 == 0;
    }
};

class InRange {
private:
    int min_, max_;
    
public:
    InRange(int min, int max) : min_(min), max_(max) {}
    
    bool operator()(int x) const {
        return x >= min_ && x <= max_;
    }
};

void demonstrate_predicates() {
    SECTION("Predicates");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Count even numbers
    IsEven is_even;
    int even_count = std::count_if(data.begin(), data.end(), is_even);
    std::cout << "Even numbers count: " << even_count << std::endl;
    
    // Find first number in range
    InRange in_range(5, 8);
    auto it = std::find_if(data.begin(), data.end(), in_range);
    if (it != data.end()) {
        std::cout << "First number in range [5,8]: " << *it << std::endl;
    }
    
    // Partition based on predicate
    auto data_copy = data;
    auto partition_point = std::partition(data_copy.begin(), data_copy.end(), is_even);
    
    std::cout << "Partitioned (even first): ";
    for (int x : data_copy) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "Partition point at position: " << (partition_point - data_copy.begin()) << std::endl;
}

// =============================================================================
// 3. STANDARD LIBRARY FUNCTORS
// =============================================================================

void demonstrate_standard_functors() {
    SECTION("Standard Library Functors");
    
    std::vector<int> data = {5, 2, 8, 1, 9, 3};
    
    // Arithmetic functors
    std::cout << "Sum using std::plus: " 
              << std::accumulate(data.begin(), data.end(), 0, std::plus<int>()) << std::endl;
    
    std::cout << "Product using std::multiplies: " 
              << std::accumulate(data.begin(), data.end(), 1, std::multiplies<int>()) << std::endl;
    
    // Comparison functors
    std::sort(data.begin(), data.end(), std::greater<int>());
    std::cout << "Sorted descending: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Logical functors
    std::vector<bool> bools = {true, false, true, false};
    bool all_true = std::accumulate(bools.begin(), bools.end(), true, std::logical_and<bool>());
    bool any_true = std::accumulate(bools.begin(), bools.end(), false, std::logical_or<bool>());
    
    std::cout << "All true: " << all_true << std::endl;
    std::cout << "Any true: " << any_true << std::endl;
}

// =============================================================================
// 4. FUNCTION ADAPTERS
// =============================================================================

void demonstrate_function_adapters() {
    SECTION("Function Adapters");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // bind1st and bind2nd (deprecated, use std::bind)
    // Using std::bind
    auto multiply_by_2 = std::bind(std::multiplies<int>(), std::placeholders::_1, 2);
    std::transform(data.begin(), data.end(), data.begin(), multiply_by_2);
    
    std::cout << "After multiplying by 2: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // not1 and not2 (deprecated, use lambda or std::not_fn)
    auto is_not_even = std::not_fn(IsEven{});
    int odd_count = std::count_if(data.begin(), data.end(), is_not_even);
    std::cout << "Odd numbers count: " << odd_count << std::endl;
    
    // mem_fn for member functions
    std::vector<std::string> strings = {"hello", "world", "test"};
    std::vector<size_t> lengths;
    std::transform(strings.begin(), strings.end(), std::back_inserter(lengths),
                   std::mem_fn(&std::string::length));
    
    std::cout << "String lengths: ";
    for (size_t len : lengths) std::cout << len << " ";
    std::cout << std::endl;
}

// =============================================================================
// 5. LAMBDA EXPRESSIONS AS FUNCTORS
// =============================================================================

void demonstrate_lambdas_as_functors() {
    SECTION("Lambda Expressions as Functors");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Simple lambda predicate
    auto is_greater_than_5 = [](int x) { return x > 5; };
    int count = std::count_if(data.begin(), data.end(), is_greater_than_5);
    std::cout << "Numbers > 5: " << count << std::endl;
    
    // Lambda with capture
    int threshold = 7;
    auto is_greater_than_threshold = [threshold](int x) { return x > threshold; };
    count = std::count_if(data.begin(), data.end(), is_greater_than_threshold);
    std::cout << "Numbers > " << threshold << ": " << count << std::endl;
    
    // Mutable lambda (stateful)
    auto counter = [count = 0](int) mutable { return ++count; };
    std::vector<int> indices(5);
    std::generate(indices.begin(), indices.end(), counter);
    
    std::cout << "Generated indices: ";
    for (int x : indices) std::cout << x << " ";
    std::cout << std::endl;
    
    // Generic lambda (C++14)
    auto print_pair = [](const auto& p) {
        std::cout << "(" << p.first << ", " << p.second << ") ";
    };
    
    std::vector<std::pair<int, std::string>> pairs = {{1, "one"}, {2, "two"}, {3, "three"}};
    std::cout << "Pairs: ";
    std::for_each(pairs.begin(), pairs.end(), print_pair);
    std::cout << std::endl;
}

// =============================================================================
// 6. PERFORMANCE CONSIDERATIONS
// =============================================================================

void performance_comparison() {
    SECTION("Performance Considerations");
    
    const size_t size = 1000000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 1);
    
    // Function pointer
    auto start = std::chrono::high_resolution_clock::now();
    bool (*func_ptr)(int) = [](int x) { return x % 2 == 0; };
    int count1 = std::count_if(data.begin(), data.end(), func_ptr);
    auto func_ptr_time = std::chrono::high_resolution_clock::now() - start;
    
    // Functor
    start = std::chrono::high_resolution_clock::now();
    IsEven functor;
    int count2 = std::count_if(data.begin(), data.end(), functor);
    auto functor_time = std::chrono::high_resolution_clock::now() - start;
    
    // Lambda
    start = std::chrono::high_resolution_clock::now();
    auto lambda = [](int x) { return x % 2 == 0; };
    int count3 = std::count_if(data.begin(), data.end(), lambda);
    auto lambda_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance comparison (counting even numbers):" << std::endl;
    std::cout << "Function pointer: " << std::chrono::duration_cast<std::chrono::microseconds>(func_ptr_time).count() << " μs" << std::endl;
    std::cout << "Functor: " << std::chrono::duration_cast<std::chrono::microseconds>(functor_time).count() << " μs" << std::endl;
    std::cout << "Lambda: " << std::chrono::duration_cast<std::chrono::microseconds>(lambda_time).count() << " μs" << std::endl;
    
    std::cout << "All results: " << count1 << ", " << count2 << ", " << count3 << std::endl;
    
    std::cout << "\nPerformance notes:" << std::endl;
    std::cout << "- Functors and lambdas are typically inlined" << std::endl;
    std::cout << "- Function pointers prevent inlining" << std::endl;
    std::cout << "- Modern compilers optimize lambdas very well" << std::endl;
}

// =============================================================================
// 7. ADVANCED FUNCTOR TECHNIQUES
// =============================================================================

template<typename Predicate>
class negate_predicate {
private:
    Predicate pred_;
    
public:
    explicit negate_predicate(Predicate pred) : pred_(pred) {}
    
    template<typename T>
    bool operator()(const T& x) const {
        return !pred_(x);
    }
};

template<typename Pred1, typename Pred2>
class and_predicate {
private:
    Pred1 pred1_;
    Pred2 pred2_;
    
public:
    and_predicate(Pred1 pred1, Pred2 pred2) : pred1_(pred1), pred2_(pred2) {}
    
    template<typename T>
    bool operator()(const T& x) const {
        return pred1_(x) && pred2_(x);
    }
};

void demonstrate_advanced_functors() {
    SECTION("Advanced Functor Techniques");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Negating predicate
    IsEven is_even;
    negate_predicate<IsEven> is_odd(is_even);
    
    int odd_count = std::count_if(data.begin(), data.end(), is_odd);
    std::cout << "Odd count using negated predicate: " << odd_count << std::endl;
    
    // Combining predicates
    InRange in_range(3, 7);
    and_predicate<IsEven, InRange> even_and_in_range(is_even, in_range);
    
    int combined_count = std::count_if(data.begin(), data.end(), even_and_in_range);
    std::cout << "Even numbers in range [3,7]: " << combined_count << std::endl;
    
    std::cout << "\nAdvantages of functors:" << std::endl;
    std::cout << "- Can hold state" << std::endl;
    std::cout << "- Inlinable for performance" << std::endl;
    std::cout << "- Type-safe" << std::endl;
    std::cout << "- Composable" << std::endl;
}

int main() {
    std::cout << "FUNCTORS AND PREDICATES TUTORIAL\n";
    std::cout << "================================\n";
    
    demonstrate_functors();
    demonstrate_predicates();
    demonstrate_standard_functors();
    demonstrate_function_adapters();
    demonstrate_lambdas_as_functors();
    performance_comparison();
    demonstrate_advanced_functors();
    
    return 0;
}

/*
KEY CONCEPTS:
- Functors are objects that can be called like functions
- Predicates return bool and are used for filtering/testing
- Standard library provides common functors (plus, less, etc.)
- Function adapters modify existing functors (bind, not_fn)
- Lambdas are modern, convenient functors
- Functors can hold state, unlike regular functions
- Performance: functors ≈ lambdas > function pointers
- Composition enables building complex logic from simple parts
*/
