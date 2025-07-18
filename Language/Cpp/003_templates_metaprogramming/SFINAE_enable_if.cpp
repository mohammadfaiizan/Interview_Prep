/*
 * SFINAE and enable_if - Substitution Failure Is Not An Error
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -g -O2 SFINAE_enable_if.cpp -o SFINAE_enable_if
 */

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include <memory>
#include <iterator>
#include <typeinfo>

#define SFINAE_INFO(msg) std::cout << "[SFINAE INFO] " << msg << std::endl
#define TEST_RESULT(expr) std::cout << "[TEST] " << #expr << " = " << (expr) << std::endl

namespace SFINAEDemo {

// ============================================================================
// 1. BASIC SFINAE WITH FUNCTION OVERLOADING
// ============================================================================

// Function overloads using SFINAE
template<typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
process_number(T value) {
    std::cout << "Processing integral number: " << value << std::endl;
    std::cout << "  Size: " << sizeof(T) << " bytes" << std::endl;
    std::cout << "  Is signed: " << std::is_signed_v<T> << std::endl;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
process_number(T value) {
    std::cout << "Processing floating-point number: " << value << std::endl;
    std::cout << "  Size: " << sizeof(T) << " bytes" << std::endl;
    std::cout << "  Precision: " << std::numeric_limits<T>::digits10 << " decimal digits" << std::endl;
}

// SFINAE with return type
template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
safe_abs(T value) {
    SFINAE_INFO("safe_abs for arithmetic types");
    return value < 0 ? -value : value;
}

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value, T>::type
safe_abs(T value) {
    SFINAE_INFO("safe_abs for non-arithmetic types (identity)");
    return value;
}

// ============================================================================
// 2. MODERN SFINAE WITH enable_if_t (C++14)
// ============================================================================

// Using enable_if_t for cleaner syntax
template<typename T>
std::enable_if_t<std::is_container_v<T>, void>
print_container(const T& container) {
    std::cout << "Container with " << container.size() << " elements: ";
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

template<typename T>
std::enable_if_t<!std::is_container_v<T>, void>
print_container(const T& value) {
    std::cout << "Single value: " << value << std::endl;
}

// SFINAE with template parameter
template<typename T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
void handle_pointer(T ptr) {
    std::cout << "Handling pointer: " << static_cast<void*>(ptr) << std::endl;
    if (ptr) {
        std::cout << "  Dereferenced value: " << *ptr << std::endl;
    } else {
        std::cout << "  Null pointer" << std::endl;
    }
}

template<typename T, std::enable_if_t<!std::is_pointer_v<T>, int> = 0>
void handle_pointer(const T& value) {
    std::cout << "Handling non-pointer value: " << value << std::endl;
}

// ============================================================================
// 3. EXPRESSION SFINAE
// ============================================================================

// Detect if type has begin() method
template<typename T>
class has_begin {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().begin(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_begin_v = has_begin<T>::value;

// Detect if type has size() method
template<typename T>
class has_size {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().size(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_size_v = has_size<T>::value;

// Detect if type has push_back method
template<typename T>
class has_push_back {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>().push_back(std::declval<typename U::value_type>()),
        std::true_type{}
    );
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_push_back_v = has_push_back<T>::value;

// Detect if type supports operator<<
template<typename T>
class is_printable {
private:
    template<typename U>
    static auto test(int) -> decltype(std::cout << std::declval<U>(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool is_printable_v = is_printable<T>::value;

// ============================================================================
// 4. SFINAE WITH MEMBER DETECTION
// ============================================================================

// Generic member detection macro
#define DEFINE_HAS_MEMBER(member_name) \
    template<typename T> \
    class has_##member_name { \
    private: \
        template<typename U> \
        static auto test(int) -> decltype(std::declval<U>().member_name, std::true_type{}); \
        template<typename> \
        static std::false_type test(...); \
    public: \
        static constexpr bool value = decltype(test<T>(0))::value; \
    }; \
    template<typename T> \
    constexpr bool has_##member_name##_v = has_##member_name<T>::value;

// Define specific member detectors
DEFINE_HAS_MEMBER(data)
DEFINE_HAS_MEMBER(empty)
DEFINE_HAS_MEMBER(clear)

// Detect member function with specific signature
template<typename T>
class has_at_method {
private:
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U>().at(std::declval<size_t>()),
        std::true_type{}
    );
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_at_method_v = has_at_method<T>::value;

// ============================================================================
// 5. SFINAE WITH ITERATOR TRAITS
// ============================================================================

// Detect iterator category
template<typename Iterator>
class is_input_iterator {
private:
    template<typename I>
    static auto test(int) -> decltype(
        typename std::iterator_traits<I>::iterator_category{},
        std::is_convertible_v<
            typename std::iterator_traits<I>::iterator_category,
            std::input_iterator_tag
        > ? std::true_type{} : std::false_type{}
    );
    
    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<Iterator>(0))::value;
};

template<typename Iterator>
constexpr bool is_input_iterator_v = is_input_iterator<Iterator>::value;

// Function template with iterator constraints
template<typename Iterator>
std::enable_if_t<is_input_iterator_v<Iterator>, void>
process_range(Iterator first, Iterator last) {
    std::cout << "Processing range with input iterator" << std::endl;
    while (first != last) {
        std::cout << *first << " ";
        ++first;
    }
    std::cout << std::endl;
}

// ============================================================================
// 6. ADVANCED SFINAE PATTERNS
// ============================================================================

// Multiple constraints with conjunction
template<typename T>
using is_numeric_container = std::conjunction<
    has_begin<T>,
    has_size<T>,
    std::is_arithmetic<typename T::value_type>
>;

template<typename T>
constexpr bool is_numeric_container_v = is_numeric_container<T>::value;

template<typename T>
std::enable_if_t<is_numeric_container_v<T>, typename T::value_type>
sum_container(const T& container) {
    using ValueType = typename T::value_type;
    ValueType sum = ValueType{};
    for (const auto& item : container) {
        sum += item;
    }
    return sum;
}

// SFINAE with disjunction (OR logic)
template<typename T>
using is_string_like = std::disjunction<
    std::is_same<T, std::string>,
    std::is_same<T, const char*>,
    std::is_same<T, char*>
>;

template<typename T>
constexpr bool is_string_like_v = is_string_like<T>::value;

template<typename T>
std::enable_if_t<is_string_like_v<T>, size_t>
get_length(const T& str) {
    if constexpr (std::is_same_v<T, std::string>) {
        return str.length();
    } else {
        return std::strlen(str);
    }
}

// ============================================================================
// 7. SFINAE WITH CONCEPTS-LIKE PROGRAMMING
// ============================================================================

// Concept-like template constraints
template<typename T>
constexpr bool Container = has_begin_v<T> && has_size_v<T>;

template<typename T>
constexpr bool Arithmetic = std::is_arithmetic_v<T>;

template<typename T>
constexpr bool Printable = is_printable_v<T>;

// Function templates with concept-like constraints
template<typename T>
std::enable_if_t<Container<T> && Printable<typename T::value_type>, void>
print_all(const T& container) {
    std::cout << "Printing container elements:" << std::endl;
    for (const auto& item : container) {
        std::cout << "  " << item << std::endl;
    }
}

template<typename T>
std::enable_if_t<Arithmetic<T>, T>
compute_square(T value) {
    return value * value;
}

// ============================================================================
// 8. SFINAE WITH VOID_T
// ============================================================================

// Using void_t for cleaner SFINAE
template<typename T, typename = void>
struct is_complete : std::false_type {};

template<typename T>
struct is_complete<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

template<typename T>
constexpr bool is_complete_v = is_complete<T>::value;

// Detect nested types
template<typename T, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type {};

template<typename T>
constexpr bool has_value_type_v = has_value_type<T>::value;

// Detect multiple members with void_t
template<typename T, typename = void>
struct is_map_like : std::false_type {};

template<typename T>
struct is_map_like<T, std::void_t<
    typename T::key_type,
    typename T::mapped_type,
    decltype(std::declval<T>().find(std::declval<typename T::key_type>()))
>> : std::true_type {};

template<typename T>
constexpr bool is_map_like_v = is_map_like<T>::value;

// ============================================================================
// 9. PRACTICAL SFINAE APPLICATIONS
// ============================================================================

// Generic serialization function
template<typename T>
std::enable_if_t<is_printable_v<T>, std::string>
serialize(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
std::enable_if_t<!is_printable_v<T>, std::string>
serialize(const T&) {
    return "non-serializable object of type: " + std::string(typeid(T).name());
}

// Generic container operations
template<typename Container>
std::enable_if_t<has_push_back_v<Container>, void>
add_element(Container& container, const typename Container::value_type& value) {
    container.push_back(value);
    std::cout << "Added element using push_back" << std::endl;
}

template<typename Container>
std::enable_if_t<!has_push_back_v<Container> && has_insert_v<Container>, void>
add_element(Container& container, const typename Container::value_type& value) {
    container.insert(value);
    std::cout << "Added element using insert" << std::endl;
}

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

void demonstrate_basic_sfinae() {
    std::cout << "\n=== BASIC SFINAE ===" << std::endl;
    
    process_number(42);
    process_number(3.14f);
    process_number(2.71828);
    
    std::cout << "safe_abs(42): " << safe_abs(42) << std::endl;
    std::cout << "safe_abs(-3.14): " << safe_abs(-3.14) << std::endl;
    std::cout << "safe_abs(string): " << safe_abs(std::string("hello")) << std::endl;
}

void demonstrate_modern_sfinae() {
    std::cout << "\n=== MODERN SFINAE ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_container(vec);
    print_container(42);
    
    int value = 100;
    int* ptr = &value;
    handle_pointer(ptr);
    handle_pointer(nullptr);
    handle_pointer(42);
}

void demonstrate_expression_sfinae() {
    std::cout << "\n=== EXPRESSION SFINAE ===" << std::endl;
    
    TEST_RESULT(has_begin_v<std::vector<int>>);
    TEST_RESULT(has_begin_v<int>);
    
    TEST_RESULT(has_size_v<std::string>);
    TEST_RESULT(has_size_v<double>);
    
    TEST_RESULT(has_push_back_v<std::vector<int>>);
    TEST_RESULT(has_push_back_v<std::array<int, 5>>);
    
    TEST_RESULT(is_printable_v<int>);
    TEST_RESULT(is_printable_v<std::vector<int>>);
}

void demonstrate_member_detection() {
    std::cout << "\n=== MEMBER DETECTION ===" << std::endl;
    
    TEST_RESULT(has_data_v<std::vector<int>>);
    TEST_RESULT(has_data_v<std::string>);
    TEST_RESULT(has_data_v<int>);
    
    TEST_RESULT(has_empty_v<std::vector<int>>);
    TEST_RESULT(has_empty_v<std::string>);
    
    TEST_RESULT(has_at_method_v<std::vector<int>>);
    TEST_RESULT(has_at_method_v<std::string>);
    TEST_RESULT(has_at_method_v<int>);
}

void demonstrate_iterator_sfinae() {
    std::cout << "\n=== ITERATOR SFINAE ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    process_range(vec.begin(), vec.end());
    
    TEST_RESULT(is_input_iterator_v<std::vector<int>::iterator>);
    TEST_RESULT(is_input_iterator_v<int*>);
    TEST_RESULT(is_input_iterator_v<int>);
}

void demonstrate_advanced_patterns() {
    std::cout << "\n=== ADVANCED PATTERNS ===" << std::endl;
    
    std::vector<int> int_vec = {1, 2, 3, 4, 5};
    std::vector<std::string> str_vec = {"a", "b", "c"};
    
    TEST_RESULT(is_numeric_container_v<decltype(int_vec)>);
    TEST_RESULT(is_numeric_container_v<decltype(str_vec)>);
    
    if constexpr (is_numeric_container_v<decltype(int_vec)>) {
        std::cout << "Sum of numeric container: " << sum_container(int_vec) << std::endl;
    }
    
    TEST_RESULT(is_string_like_v<std::string>);
    TEST_RESULT(is_string_like_v<const char*>);
    TEST_RESULT(is_string_like_v<int>);
    
    std::string str = "hello";
    const char* cstr = "world";
    std::cout << "String length: " << get_length(str) << std::endl;
    std::cout << "C-string length: " << get_length(cstr) << std::endl;
}

void demonstrate_concept_like() {
    std::cout << "\n=== CONCEPT-LIKE PROGRAMMING ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_all(vec);
    
    std::cout << "Square of 5: " << compute_square(5) << std::endl;
    std::cout << "Square of 3.14: " << compute_square(3.14) << std::endl;
}

void demonstrate_void_t() {
    std::cout << "\n=== VOID_T PATTERNS ===" << std::endl;
    
    TEST_RESULT(is_complete_v<int>);
    TEST_RESULT(has_value_type_v<std::vector<int>>);
    TEST_RESULT(has_value_type_v<int>);
    
    TEST_RESULT(is_map_like_v<std::map<int, std::string>>);
    TEST_RESULT(is_map_like_v<std::vector<int>>);
}

void demonstrate_practical_applications() {
    std::cout << "\n=== PRACTICAL APPLICATIONS ===" << std::endl;
    
    std::cout << "Serialize int: " << serialize(42) << std::endl;
    std::cout << "Serialize string: " << serialize(std::string("hello")) << std::endl;
    
    struct NonPrintable {};
    std::cout << "Serialize non-printable: " << serialize(NonPrintable{}) << std::endl;
    
    std::vector<int> vec;
    add_element(vec, 42);
    std::cout << "Vector size after adding: " << vec.size() << std::endl;
}

} // namespace SFINAEDemo

int main() {
    std::cout << "=== SFINAE AND ENABLE_IF TUTORIAL ===" << std::endl;
    
    try {
        SFINAEDemo::demonstrate_basic_sfinae();
        SFINAEDemo::demonstrate_modern_sfinae();
        SFINAEDemo::demonstrate_expression_sfinae();
        SFINAEDemo::demonstrate_member_detection();
        SFINAEDemo::demonstrate_iterator_sfinae();
        SFINAEDemo::demonstrate_advanced_patterns();
        SFINAEDemo::demonstrate_concept_like();
        SFINAEDemo::demonstrate_void_t();
        SFINAEDemo::demonstrate_practical_applications();
        
        std::cout << "\n=== TUTORIAL COMPLETED ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
