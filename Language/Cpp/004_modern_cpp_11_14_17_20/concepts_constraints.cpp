/*
 * MODERN C++ - CONCEPTS AND CONSTRAINTS (C++20)
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra concepts_constraints.cpp -o concepts_constraints
 */

#include <iostream>
#include <concepts>
#include <type_traits>
#include <vector>
#include <string>
#include <iterator>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC CONCEPTS
// =============================================================================

// Simple concept definition
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Concept with requirements
template<typename T>
concept Printable = requires(T t) {
    std::cout << t;  // Must be printable to cout
};

// Function using concepts
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// Alternative syntax
template<typename T>
requires Numeric<T>
T multiply(T a, T b) {
    return a * b;
}

void demonstrate_basic_concepts() {
    SECTION("Basic Concepts");
    
    // Works with numeric types
    std::cout << "add(5, 3) = " << add(5, 3) << std::endl;
    std::cout << "add(2.5, 1.7) = " << add(2.5, 1.7) << std::endl;
    
    // multiply with requires clause
    std::cout << "multiply(4, 6) = " << multiply(4, 6) << std::endl;
    
    // This would fail to compile:
    // add("hello", "world");  // Error: std::string is not Numeric
    
    // Test printable concept
    auto print_if_possible = []<Printable T>(T value) {
        std::cout << "Printing: " << value << std::endl;
    };
    
    print_if_possible(42);
    print_if_possible("Hello Concepts!");
    print_if_possible(3.14);
}

// =============================================================================
// 2. STANDARD LIBRARY CONCEPTS
// =============================================================================

void demonstrate_standard_concepts() {
    SECTION("Standard Library Concepts");
    
    // std::integral concept
    auto process_integral = []<std::integral T>(T value) {
        std::cout << "Processing integral: " << value << std::endl;
        return value * 2;
    };
    
    std::cout << "Result: " << process_integral(42) << std::endl;
    // process_integral(3.14);  // Error: double is not integral
    
    // std::convertible_to concept
    auto convert_and_print = []<typename From, typename To>
        (From from) requires std::convertible_to<From, To> {
        To converted = static_cast<To>(from);
        std::cout << "Converted " << from << " to " << converted << std::endl;
        return converted;
    };
    
    convert_and_print<int, double>(42);
    convert_and_print<double, int>(3.14);
    
    // std::same_as concept
    auto same_type_operation = []<typename T, typename U>
        (T a, U b) requires std::same_as<T, U> {
        return a + b;
    };
    
    std::cout << "Same types: " << same_type_operation(5, 10) << std::endl;
    // same_type_operation(5, 3.14);  // Error: different types
}

// =============================================================================
// 3. CUSTOM CONCEPTS WITH REQUIREMENTS
// =============================================================================

// Concept checking for specific operations
template<typename T>
concept Incrementable = requires(T t) {
    ++t;     // Must support pre-increment
    t++;     // Must support post-increment
};

template<typename T>
concept Container = requires(T t) {
    t.begin();
    t.end();
    t.size();
    typename T::value_type;
};

template<typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
    { a == b } -> std::convertible_to<bool>;
};

// Concept with nested requirements
template<typename T>
concept Sortable = Container<T> && requires(T container) {
    requires Comparable<typename T::value_type>;
};

void demonstrate_custom_concepts() {
    SECTION("Custom Concepts with Requirements");
    
    // Test Incrementable
    auto increment_twice = []<Incrementable T>(T value) {
        ++value;
        value++;
        return value;
    };
    
    int i = 5;
    std::cout << "Incremented: " << increment_twice(i) << std::endl;
    
    // Test Container
    auto print_container_info = []<Container C>(const C& container) {
        std::cout << "Container size: " << container.size() << std::endl;
        std::cout << "Container contents: ";
        for (const auto& item : container) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    };
    
    std::vector<int> vec{1, 2, 3, 4, 5};
    print_container_info(vec);
    
    std::string str = "hello";
    print_container_info(str);  // string is also a container
    
    // Test Sortable (would work with sort algorithm)
    static_assert(Sortable<std::vector<int>>);
    static_assert(Sortable<std::vector<std::string>>);
    std::cout << "Vector<int> is sortable: " << std::boolalpha 
              << Sortable<std::vector<int>> << std::endl;
}

// =============================================================================
// 4. CONCEPT COMPOSITION AND LOGIC
// =============================================================================

template<typename T>
concept SignedIntegral = std::integral<T> && std::signed_integral<T>;

template<typename T>
concept UnsignedIntegral = std::integral<T> && std::unsigned_integral<T>;

template<typename T>
concept NumericContainer = Container<T> && Numeric<typename T::value_type>;

// Disjunction (OR)
template<typename T>
concept IntegerOrFloat = std::integral<T> || std::floating_point<T>;

void demonstrate_concept_composition() {
    SECTION("Concept Composition and Logic");
    
    auto process_signed = []<SignedIntegral T>(T value) {
        std::cout << "Processing signed integral: " << value << std::endl;
        return value < 0 ? -value : value;  // abs
    };
    
    process_signed(-42);
    // process_signed(42u);  // Error: unsigned int not SignedIntegral
    
    auto process_unsigned = []<UnsignedIntegral T>(T value) {
        std::cout << "Processing unsigned integral: " << value << std::endl;
        return value + 1;
    };
    
    process_unsigned(42u);
    
    auto process_numeric_container = []<NumericContainer C>(const C& container) {
        std::cout << "Numeric container with " << container.size() << " elements" << std::endl;
        typename C::value_type sum{};
        for (const auto& val : container) {
            sum += val;
        }
        return sum;
    };
    
    std::vector<double> numbers{1.1, 2.2, 3.3, 4.4};
    std::cout << "Sum: " << process_numeric_container(numbers) << std::endl;
    
    auto process_number = []<IntegerOrFloat T>(T value) {
        std::cout << "Processing number (int or float): " << value << std::endl;
        return value * 2;
    };
    
    std::cout << "Result: " << process_number(21) << std::endl;
    std::cout << "Result: " << process_number(3.14) << std::endl;
}

// =============================================================================
// 5. REQUIRES EXPRESSIONS AND CLAUSES
// =============================================================================

// Complex requires expression
template<typename T>
concept Drawable = requires(T obj) {
    obj.draw();                    // Simple requirement
    { obj.get_area() } -> std::convertible_to<double>;  // Compound requirement
    typename T::coordinate_type;   // Type requirement
    requires std::is_move_constructible_v<T>;  // Nested requirement
};

// Requires clause at the end
template<typename T>
auto safe_divide(T a, T b) -> T 
    requires std::floating_point<T> 
{
    if (b == T{0}) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

// Multiple constraints
template<typename Iterator>
auto distance(Iterator first, Iterator last) 
    requires std::input_iterator<Iterator>
{
    if constexpr (std::random_access_iterator<Iterator>) {
        return last - first;  // O(1) for random access
    } else {
        std::iter_difference_t<Iterator> count = 0;
        while (first != last) {
            ++first;
            ++count;
        }
        return count;  // O(n) for other iterators
    }
}

void demonstrate_requires_expressions() {
    SECTION("Requires Expressions and Clauses");
    
    // Test safe_divide
    try {
        std::cout << "3.14 / 2.0 = " << safe_divide(3.14, 2.0) << std::endl;
        std::cout << "5.0 / 0.0 = " << safe_divide(5.0, 0.0) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    // Test custom distance
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::cout << "Vector distance: " << distance(vec.begin(), vec.end()) << std::endl;
    
    // Demonstrate concept checking at compile time
    std::cout << "Checking concepts:" << std::endl;
    std::cout << "int is Numeric: " << std::boolalpha << Numeric<int> << std::endl;
    std::cout << "string is Numeric: " << std::boolalpha << Numeric<std::string> << std::endl;
    std::cout << "vector<int> is Container: " << std::boolalpha << Container<std::vector<int>> << std::endl;
}

// =============================================================================
// 6. ABBREVIATED FUNCTION TEMPLATES
// =============================================================================

// Abbreviated function template syntax
void print_comparable(Comparable auto value) {
    std::cout << "Comparable value: " << value << std::endl;
}

auto add_numbers(Numeric auto a, Numeric auto b) {
    return a + b;
}

// Mixed abbreviated and traditional
template<Container C>
void process_container(C&& container, Printable auto processor) {
    std::cout << "Processing container of size: " << container.size() << std::endl;
    for (const auto& item : container) {
        processor(item);
    }
}

void demonstrate_abbreviated_templates() {
    SECTION("Abbreviated Function Templates");
    
    print_comparable(42);
    print_comparable("hello");
    print_comparable(3.14);
    
    std::cout << "add_numbers(5, 3) = " << add_numbers(5, 3) << std::endl;
    std::cout << "add_numbers(2.5, 1.7) = " << add_numbers(2.5, 1.7) << std::endl;
    
    // Mixed types with concept constraints
    std::cout << "add_numbers(5, 3.14) = " << add_numbers(5, 3.14) << std::endl;
    
    std::vector<int> numbers{1, 2, 3, 4, 5};
    process_container(numbers, [](int n) { 
        std::cout << n * n << " "; 
    });
    std::cout << std::endl;
}

// =============================================================================
// 7. REAL-WORLD EXAMPLE: GENERIC ALGORITHM
// =============================================================================

template<typename T>
concept Serializable = requires(T obj) {
    { obj.serialize() } -> std::convertible_to<std::string>;
    obj.deserialize(std::string{});
};

// Generic serialization function
template<Container C>
requires Serializable<typename C::value_type>
std::string serialize_container(const C& container) {
    std::string result = "[";
    bool first = true;
    for (const auto& item : container) {
        if (!first) result += ",";
        result += item.serialize();
        first = false;
    }
    result += "]";
    return result;
}

// Example serializable class
class Point {
public:
    int x, y;
    
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    
    std::string serialize() const {
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }
    
    void deserialize(const std::string& data) {
        // Simple parsing (not robust)
        // In real code, use proper parsing
        std::cout << "Deserializing: " << data << std::endl;
    }
};

void demonstrate_real_world_example() {
    SECTION("Real-World Example: Generic Algorithm");
    
    std::vector<Point> points{{1, 2}, {3, 4}, {5, 6}};
    
    static_assert(Serializable<Point>);
    static_assert(Container<std::vector<Point>>);
    
    std::string serialized = serialize_container(points);
    std::cout << "Serialized points: " << serialized << std::endl;
    
    // This would fail to compile if Point wasn't Serializable:
    // std::vector<int> ints{1, 2, 3};
    // serialize_container(ints);  // Error: int is not Serializable
}

int main() {
    std::cout << "CONCEPTS AND CONSTRAINTS (C++20) TUTORIAL\n";
    std::cout << "=========================================\n";
    
    demonstrate_basic_concepts();
    demonstrate_standard_concepts();
    demonstrate_custom_concepts();
    demonstrate_concept_composition();
    demonstrate_requires_expressions();
    demonstrate_abbreviated_templates();
    demonstrate_real_world_example();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Concepts define named requirements for template parameters
- Better error messages than SFINAE
- Standard concepts: integral, floating_point, convertible_to, etc.
- Custom concepts with requires expressions
- Concept composition with &&, ||, !
- Abbreviated function template syntax: void func(Concept auto param)
- Requires clauses for additional constraints
- Compile-time concept checking with concept_name<Type>
- Significant improvement in template programming ergonomics
- C++20 feature requiring modern compiler support
*/
