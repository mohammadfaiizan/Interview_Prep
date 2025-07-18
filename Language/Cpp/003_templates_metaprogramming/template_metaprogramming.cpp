/*
 * TEMPLATE METAPROGRAMMING - Advanced C++ Compile-time Programming
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -g -O2 template_metaprogramming.cpp -o template_metaprogramming
 */

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include <tuple>
#include <array>
#include <typeinfo>

#define META_INFO(msg) std::cout << "[META INFO] " << msg << std::endl
#define COMPILE_TIME_VALUE(expr) std::cout << "[COMPILE-TIME] " << #expr << " = " << (expr) << std::endl

namespace TemplateMetaprogramming {

// ============================================================================
// 1. COMPILE-TIME ARITHMETIC
// ============================================================================

// Factorial computation at compile time
template<int N>
struct factorial {
    static constexpr int value = N * factorial<N-1>::value;
};

template<>
struct factorial<0> {
    static constexpr int value = 1;
};

// Fibonacci sequence at compile time
template<int N>
struct fibonacci {
    static constexpr int value = fibonacci<N-1>::value + fibonacci<N-2>::value;
};

template<>
struct fibonacci<0> {
    static constexpr int value = 0;
};

template<>
struct fibonacci<1> {
    static constexpr int value = 1;
};

// Power computation at compile time
template<int Base, int Exp>
struct power {
    static constexpr int value = Base * power<Base, Exp-1>::value;
};

template<int Base>
struct power<Base, 0> {
    static constexpr int value = 1;
};

// GCD computation at compile time
template<int A, int B>
struct gcd {
    static constexpr int value = gcd<B, A % B>::value;
};

template<int A>
struct gcd<A, 0> {
    static constexpr int value = A;
};

// ============================================================================
// 2. TYPE MANIPULATION
// ============================================================================

// Remove all qualifiers from a type
template<typename T>
struct remove_all {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<typename T>
using remove_all_t = typename remove_all<T>::type;

// Add pointer to type
template<typename T>
struct add_pointer_safe {
    using type = typename std::add_pointer<T>::type;
};

template<typename T>
using add_pointer_safe_t = typename add_pointer_safe<T>::type;

// Type list manipulation
template<typename... Types>
struct type_list {
    static constexpr size_t size = sizeof...(Types);
};

// Get first type from type list
template<typename List>
struct front;

template<typename Head, typename... Tail>
struct front<type_list<Head, Tail...>> {
    using type = Head;
};

template<typename List>
using front_t = typename front<List>::type;

// Get tail of type list
template<typename List>
struct tail;

template<typename Head, typename... Tail>
struct tail<type_list<Head, Tail...>> {
    using type = type_list<Tail...>;
};

template<typename List>
using tail_t = typename tail<List>::type;

// Append type to type list
template<typename List, typename T>
struct append;

template<typename... Types, typename T>
struct append<type_list<Types...>, T> {
    using type = type_list<Types..., T>;
};

template<typename List, typename T>
using append_t = typename append<List, T>::type;

// ============================================================================
// 3. CONDITIONAL COMPILATION
// ============================================================================

// Conditional type selection
template<bool Condition, typename TrueType, typename FalseType>
struct conditional_type {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct conditional_type<false, TrueType, FalseType> {
    using type = FalseType;
};

template<bool Condition, typename TrueType, typename FalseType>
using conditional_type_t = typename conditional_type<Condition, TrueType, FalseType>::type;

// Enable if implementation
template<bool Condition, typename T = void>
struct enable_if_custom {};

template<typename T>
struct enable_if_custom<true, T> {
    using type = T;
};

template<bool Condition, typename T = void>
using enable_if_custom_t = typename enable_if_custom<Condition, T>::type;

// ============================================================================
// 4. TYPE TRAITS
// ============================================================================

// Check if type is container-like
template<typename T, typename = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<T, std::void_t<
    typename T::value_type,
    typename T::iterator,
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    decltype(std::declval<T>().size())
>> : std::true_type {};

template<typename T>
constexpr bool is_container_v = is_container<T>::value;

// Check if type has specific member function
template<typename T, typename = void>
struct has_push_back : std::false_type {};

template<typename T>
struct has_push_back<T, std::void_t<
    decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))
>> : std::true_type {};

template<typename T>
constexpr bool has_push_back_v = has_push_back<T>::value;

// Check if type is callable
template<typename T, typename... Args>
struct is_callable {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>()(std::declval<Args>()...), std::true_type{});
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename... Args>
constexpr bool is_callable_v = is_callable<T, Args...>::value;

// ============================================================================
// 5. TEMPLATE RECURSION AND ITERATION
// ============================================================================

// Tuple iteration at compile time
template<size_t Index, typename Tuple>
struct tuple_printer {
    static void print(const Tuple& t) {
        tuple_printer<Index - 1, Tuple>::print(t);
        std::cout << ", " << std::get<Index>(t);
    }
};

template<typename Tuple>
struct tuple_printer<0, Tuple> {
    static void print(const Tuple& t) {
        std::cout << std::get<0>(t);
    }
};

template<typename... Args>
void print_tuple(const std::tuple<Args...>& t) {
    std::cout << "(";
    if constexpr (sizeof...(Args) > 0) {
        tuple_printer<sizeof...(Args) - 1, std::tuple<Args...>>::print(t);
    }
    std::cout << ")" << std::endl;
}

// Compile-time string length
template<size_t N>
constexpr size_t string_length(const char (&)[N]) {
    return N - 1; // Subtract null terminator
}

// Compile-time array sum
template<typename T, size_t N>
constexpr T array_sum(const T (&arr)[N]) {
    T sum = T{};
    for (size_t i = 0; i < N; ++i) {
        sum += arr[i];
    }
    return sum;
}

// ============================================================================
// 6. SFINAE AND EXPRESSION SFINAE
// ============================================================================

// SFINAE to detect operator<<
template<typename T>
class has_stream_operator {
private:
    template<typename U>
    static auto test(int) -> decltype(std::cout << std::declval<U>(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_stream_operator_v = has_stream_operator<T>::value;

// SFINAE-friendly print function
template<typename T>
enable_if_custom_t<has_stream_operator_v<T>, void>
safe_print(const T& value) {
    std::cout << "Printable: " << value << std::endl;
}

template<typename T>
enable_if_custom_t<!has_stream_operator_v<T>, void>
safe_print(const T&) {
    std::cout << "Non-printable type: " << typeid(T).name() << std::endl;
}

// ============================================================================
// 7. TEMPLATE SPECIALIZATION PATTERNS
// ============================================================================

// Primary template for type properties
template<typename T>
struct type_properties {
    static constexpr bool is_numeric = std::is_arithmetic_v<T>;
    static constexpr bool is_pointer = std::is_pointer_v<T>;
    static constexpr size_t size = sizeof(T);
    static constexpr const char* category = "unknown";
};

// Specialization for integral types
template<typename T>
struct type_properties<T> {
    static constexpr bool is_numeric = true;
    static constexpr bool is_pointer = false;
    static constexpr size_t size = sizeof(T);
    static constexpr const char* category = "integral";
};

// Template specialization for pointers
template<typename T>
struct type_properties<T*> {
    static constexpr bool is_numeric = false;
    static constexpr bool is_pointer = true;
    static constexpr size_t size = sizeof(T*);
    static constexpr const char* category = "pointer";
};

// ============================================================================
// 8. ADVANCED METAPROGRAMMING TECHNIQUES
// ============================================================================

// Compile-time string hashing
constexpr uint32_t fnv1a_hash(const char* str, size_t count) {
    constexpr uint32_t FNV_OFFSET_BASIS = 2166136261U;
    constexpr uint32_t FNV_PRIME = 16777619U;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < count; ++i) {
        hash ^= static_cast<uint32_t>(str[i]);
        hash *= FNV_PRIME;
    }
    return hash;
}

constexpr uint32_t operator""_hash(const char* str, size_t len) {
    return fnv1a_hash(str, len);
}

// Template-based state machine
template<int State>
struct StateMachine {
    static constexpr int current_state = State;
    
    template<int NextState>
    using transition = StateMachine<NextState>;
    
    static void print_state() {
        std::cout << "Current state: " << current_state << std::endl;
    }
};

// Compile-time sorting (selection sort)
template<int... Values>
struct sort_values;

template<>
struct sort_values<> {
    using type = std::integer_sequence<int>;
};

template<int Value>
struct sort_values<Value> {
    using type = std::integer_sequence<int, Value>;
};

// Helper to find minimum value
template<int First, int... Rest>
struct min_value {
    static constexpr int value = std::min(First, min_value<Rest...>::value);
};

template<int Value>
struct min_value<Value> {
    static constexpr int value = Value;
};

// ============================================================================
// 9. CONSTEXPR METAPROGRAMMING (C++14/17)
// ============================================================================

// Constexpr factorial
constexpr int factorial_constexpr(int n) {
    return (n <= 1) ? 1 : n * factorial_constexpr(n - 1);
}

// Constexpr string operations
constexpr bool string_equal(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

constexpr size_t string_length_constexpr(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

// Constexpr array operations
template<typename T, size_t N>
constexpr std::array<T, N> make_array(T value) {
    std::array<T, N> arr{};
    for (size_t i = 0; i < N; ++i) {
        arr[i] = value;
    }
    return arr;
}

// ============================================================================
// 10. TEMPLATE METAPROGRAMMING UTILITIES
// ============================================================================

// Type-to-string conversion (simplified)
template<typename T>
constexpr const char* type_name_simple() {
    if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else if constexpr (std::is_same_v<T, float>) return "float";
    else if constexpr (std::is_same_v<T, char>) return "char";
    else if constexpr (std::is_same_v<T, bool>) return "bool";
    else return "unknown";
}

// Compile-time type list operations
template<typename... Types>
constexpr size_t count_types() {
    return sizeof...(Types);
}

template<typename T, typename... Types>
constexpr bool contains_type() {
    return (std::is_same_v<T, Types> || ...);
}

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

void demonstrate_compile_time_arithmetic() {
    std::cout << "\n=== COMPILE-TIME ARITHMETIC ===" << std::endl;
    
    COMPILE_TIME_VALUE(factorial<5>::value);
    COMPILE_TIME_VALUE(factorial<10>::value);
    
    COMPILE_TIME_VALUE(fibonacci<10>::value);
    COMPILE_TIME_VALUE(fibonacci<15>::value);
    
    COMPILE_TIME_VALUE(power<2, 8>::value);
    COMPILE_TIME_VALUE(power<3, 4>::value);
    
    COMPILE_TIME_VALUE(gcd<48, 18>::value);
    COMPILE_TIME_VALUE(gcd<100, 75>::value);
}

void demonstrate_type_manipulation() {
    std::cout << "\n=== TYPE MANIPULATION ===" << std::endl;
    
    using list1 = type_list<int, double, std::string>;
    using list2 = append_t<list1, bool>;
    
    std::cout << "Original list size: " << list1::size << std::endl;
    std::cout << "Extended list size: " << list2::size << std::endl;
    
    using first_type = front_t<list1>;
    std::cout << "First type: " << typeid(first_type).name() << std::endl;
    
    using tail_types = tail_t<list1>;
    std::cout << "Tail size: " << tail_types::size << std::endl;
}

void demonstrate_type_traits() {
    std::cout << "\n=== CUSTOM TYPE TRAITS ===" << std::endl;
    
    std::cout << "std::vector<int> is container: " << is_container_v<std::vector<int>> << std::endl;
    std::cout << "int is container: " << is_container_v<int> << std::endl;
    
    std::cout << "std::vector<int> has push_back: " << has_push_back_v<std::vector<int>> << std::endl;
    std::cout << "std::array<int, 5> has push_back: " << has_push_back_v<std::array<int, 5>> << std::endl;
    
    auto lambda = [](int x) { return x * 2; };
    std::cout << "Lambda is callable with int: " << is_callable_v<decltype(lambda), int> << std::endl;
    std::cout << "int is callable with int: " << is_callable_v<int, int> << std::endl;
}

void demonstrate_template_recursion() {
    std::cout << "\n=== TEMPLATE RECURSION ===" << std::endl;
    
    auto tuple = std::make_tuple(42, "hello", 3.14, true);
    std::cout << "Tuple contents: ";
    print_tuple(tuple);
    
    constexpr char test_str[] = "Hello World";
    COMPILE_TIME_VALUE(string_length(test_str));
    
    constexpr int arr[] = {1, 2, 3, 4, 5};
    COMPILE_TIME_VALUE(array_sum(arr));
}

void demonstrate_sfinae() {
    std::cout << "\n=== SFINAE TECHNIQUES ===" << std::endl;
    
    safe_print(42);
    safe_print(std::string("hello"));
    
    struct NonPrintable {};
    safe_print(NonPrintable{});
    
    std::cout << "int has stream operator: " << has_stream_operator_v<int> << std::endl;
    std::cout << "NonPrintable has stream operator: " << has_stream_operator_v<NonPrintable> << std::endl;
}

void demonstrate_constexpr_metaprogramming() {
    std::cout << "\n=== CONSTEXPR METAPROGRAMMING ===" << std::endl;
    
    COMPILE_TIME_VALUE(factorial_constexpr(5));
    COMPILE_TIME_VALUE(factorial_constexpr(10));
    
    constexpr bool equal = string_equal("hello", "hello");
    constexpr bool not_equal = string_equal("hello", "world");
    COMPILE_TIME_VALUE(equal);
    COMPILE_TIME_VALUE(not_equal);
    
    COMPILE_TIME_VALUE(string_length_constexpr("Hello World"));
    
    constexpr auto arr = make_array<int, 5>(42);
    std::cout << "Constexpr array: ";
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

void demonstrate_advanced_techniques() {
    std::cout << "\n=== ADVANCED TECHNIQUES ===" << std::endl;
    
    // Compile-time string hashing
    constexpr uint32_t hash1 = "hello"_hash;
    constexpr uint32_t hash2 = "world"_hash;
    COMPILE_TIME_VALUE(hash1);
    COMPILE_TIME_VALUE(hash2);
    
    // State machine
    using initial_state = StateMachine<0>;
    using next_state = initial_state::transition<1>;
    using final_state = next_state::transition<2>;
    
    initial_state::print_state();
    next_state::print_state();
    final_state::print_state();
    
    // Type utilities
    std::cout << "Type count: " << count_types<int, double, std::string>() << std::endl;
    std::cout << "Contains int: " << contains_type<int, int, double, std::string>() << std::endl;
    std::cout << "Contains bool: " << contains_type<bool, int, double, std::string>() << std::endl;
    
    std::cout << "Type name (int): " << type_name_simple<int>() << std::endl;
    std::cout << "Type name (double): " << type_name_simple<double>() << std::endl;
}

} // namespace TemplateMetaprogramming

int main() {
    std::cout << "=== TEMPLATE METAPROGRAMMING TUTORIAL ===" << std::endl;
    
    try {
        TemplateMetaprogramming::demonstrate_compile_time_arithmetic();
        TemplateMetaprogramming::demonstrate_type_manipulation();
        TemplateMetaprogramming::demonstrate_type_traits();
        TemplateMetaprogramming::demonstrate_template_recursion();
        TemplateMetaprogramming::demonstrate_sfinae();
        TemplateMetaprogramming::demonstrate_constexpr_metaprogramming();
        TemplateMetaprogramming::demonstrate_advanced_techniques();
        
        std::cout << "\n=== TUTORIAL COMPLETED ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
