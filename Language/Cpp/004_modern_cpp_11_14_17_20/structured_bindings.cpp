/*
 * MODERN C++ - STRUCTURED BINDINGS (C++17)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra structured_bindings.cpp -o structured_bindings
 */

#include <iostream>
#include <tuple>
#include <utility>
#include <array>
#include <map>
#include <string>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC STRUCTURED BINDINGS
// =============================================================================

void demonstrate_basic_bindings() {
    SECTION("Basic Structured Bindings");
    
    // With std::pair
    std::pair<int, std::string> p{42, "hello"};
    auto [value, text] = p;
    std::cout << "Pair: " << value << ", " << text << std::endl;
    
    // With std::tuple
    std::tuple<int, double, std::string> t{1, 2.5, "world"};
    auto [i, d, s] = t;
    std::cout << "Tuple: " << i << ", " << d << ", " << s << std::endl;
    
    // With arrays
    int arr[3] = {1, 2, 3};
    auto [a, b, c] = arr;
    std::cout << "Array: " << a << ", " << b << ", " << c << std::endl;
    
    // With std::array
    std::array<int, 2> std_arr{10, 20};
    auto [first, second] = std_arr;
    std::cout << "std::array: " << first << ", " << second << std::endl;
}

// =============================================================================
// 2. STRUCTURED BINDINGS WITH REFERENCES
// =============================================================================

void demonstrate_reference_bindings() {
    SECTION("Reference Bindings");
    
    std::pair<int, std::string> p{100, "modify"};
    
    // By reference - can modify original
    auto& [val, str] = p;
    val = 200;
    str = "modified";
    
    std::cout << "Original pair: " << p.first << ", " << p.second << std::endl;
    
    // Const reference
    const auto& [cval, cstr] = p;
    std::cout << "Const ref: " << cval << ", " << cstr << std::endl;
    
    // Move semantics
    std::pair<int, std::string> p2{300, "move_me"};
    auto [moved_val, moved_str] = std::move(p2);
    std::cout << "Moved: " << moved_val << ", " << moved_str << std::endl;
    std::cout << "Original after move: " << p2.first << ", " << p2.second << std::endl;
}

// =============================================================================
// 3. STRUCTURED BINDINGS WITH CUSTOM TYPES
// =============================================================================

struct Point {
    int x, y;
};

// For structured bindings to work with custom types,
// we need to provide structured binding support
template<std::size_t I>
constexpr auto get(const Point& p) {
    if constexpr (I == 0) return p.x;
    else if constexpr (I == 1) return p.y;
}

// Specialize std::tuple_size and std::tuple_element
namespace std {
    template<>
    struct tuple_size<Point> : integral_constant<size_t, 2> {};
    
    template<size_t I>
    struct tuple_element<I, Point> {
        using type = int;
    };
}

void demonstrate_custom_types() {
    SECTION("Custom Types with Structured Bindings");
    
    Point pt{10, 20};
    auto [x, y] = pt;
    std::cout << "Point: x=" << x << ", y=" << y << std::endl;
    
    // Can also use with references
    auto& [rx, ry] = pt;
    rx = 30;
    ry = 40;
    std::cout << "Modified point: x=" << pt.x << ", y=" << pt.y << std::endl;
}

// =============================================================================
// 4. STRUCTURED BINDINGS WITH CONTAINERS
// =============================================================================

void demonstrate_container_bindings() {
    SECTION("Container Iteration with Structured Bindings");
    
    std::map<std::string, int> scores{
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };
    
    // Iterate over map with structured bindings
    std::cout << "Scores:" << std::endl;
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
    
    // Modify values
    for (auto& [name, score] : scores) {
        if (name == "Bob") {
            score = 90;  // Boost Bob's score
        }
    }
    
    std::cout << "\nUpdated scores:" << std::endl;
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
}

// =============================================================================
// 5. PRACTICAL EXAMPLES
// =============================================================================

// Function returning multiple values
std::tuple<bool, int, std::string> divide_with_info(int a, int b) {
    if (b == 0) {
        return {false, 0, "Division by zero"};
    }
    return {true, a / b, "Success"};
}

// Function returning coordinates
std::pair<double, double> get_coordinates() {
    return {3.14, 2.71};
}

void demonstrate_practical_examples() {
    SECTION("Practical Examples");
    
    // Multiple return values
    auto [success, result, message] = divide_with_info(10, 3);
    if (success) {
        std::cout << "Division result: " << result << std::endl;
    } else {
        std::cout << "Error: " << message << std::endl;
    }
    
    // Coordinates
    auto [x, y] = get_coordinates();
    std::cout << "Coordinates: (" << x << ", " << y << ")" << std::endl;
    
    // With algorithms
    std::map<int, std::string> data{{1, "one"}, {2, "two"}, {3, "three"}};
    auto [iter, inserted] = data.insert({4, "four"});
    
    if (inserted) {
        std::cout << "Inserted: " << iter->first << " -> " << iter->second << std::endl;
    }
    
    // Try to insert duplicate
    auto [iter2, inserted2] = data.insert({2, "duplicate"});
    if (!inserted2) {
        std::cout << "Key already exists: " << iter2->first << " -> " << iter2->second << std::endl;
    }
}

// =============================================================================
// 6. NESTED STRUCTURED BINDINGS
// =============================================================================

void demonstrate_nested_bindings() {
    SECTION("Nested Structured Bindings");
    
    // Nested tuples
    std::tuple<std::pair<int, int>, std::string> nested{
        {10, 20}, "nested"
    };
    
    auto [coords, description] = nested;
    auto [x, y] = coords;
    
    std::cout << "Nested: (" << x << ", " << y << ") - " << description << std::endl;
    
    // Array of pairs
    std::array<std::pair<int, int>, 2> points{{{1, 2}, {3, 4}}};
    
    for (const auto& [x, y] : points) {
        std::cout << "Point: (" << x << ", " << y << ")" << std::endl;
    }
}

int main() {
    std::cout << "STRUCTURED BINDINGS (C++17) TUTORIAL\n";
    std::cout << "====================================\n";
    
    demonstrate_basic_bindings();
    demonstrate_reference_bindings();
    demonstrate_custom_types();
    demonstrate_container_bindings();
    demonstrate_practical_examples();
    demonstrate_nested_bindings();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- auto [a, b, c] = tuple_like_object;
- Works with std::pair, std::tuple, arrays, std::array
- Can use references: auto& [a, b] = obj;
- Can use const: const auto& [a, b] = obj;
- Great for range-based for loops with maps
- Custom types need get<I>() and tuple_size/tuple_element specializations
- Improves readability when dealing with multiple return values
- C++17 feature - requires modern compiler support
*/
