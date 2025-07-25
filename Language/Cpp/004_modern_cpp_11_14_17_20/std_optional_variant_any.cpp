/*
 * MODERN C++ - STD::OPTIONAL, STD::VARIANT, STD::ANY (C++17)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra std_optional_variant_any.cpp -o optional_variant_any
 */

#include <iostream>
#include <optional>
#include <variant>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. STD::OPTIONAL - NULLABLE VALUES
// =============================================================================

std::optional<int> safe_divide(int a, int b) {
    if (b == 0) {
        return std::nullopt;  // No value
    }
    return a / b;  // Has value
}

std::optional<std::string> find_user(int id) {
    if (id == 123) {
        return "Alice";
    } else if (id == 456) {
        return "Bob";
    }
    return std::nullopt;  // User not found
}

void demonstrate_optional() {
    SECTION("std::optional - Nullable Values");
    
    // Basic usage
    std::optional<int> opt1 = 42;
    std::optional<int> opt2;  // Empty
    
    std::cout << "opt1 has value: " << opt1.has_value() << std::endl;
    std::cout << "opt2 has value: " << opt2.has_value() << std::endl;
    
    if (opt1) {  // Implicit bool conversion
        std::cout << "opt1 value: " << *opt1 << std::endl;  // Dereference
        std::cout << "opt1 value: " << opt1.value() << std::endl;  // value() method
    }
    
    // Safe division example
    auto result1 = safe_divide(10, 2);
    auto result2 = safe_divide(10, 0);
    
    if (result1) {
        std::cout << "10 / 2 = " << *result1 << std::endl;
    }
    
    if (!result2) {
        std::cout << "Division by zero detected" << std::endl;
    }
    
    // Using value_or for default values
    std::cout << "Result or default: " << result2.value_or(-1) << std::endl;
    
    // User lookup example
    auto user1 = find_user(123);
    auto user2 = find_user(999);
    
    std::cout << "User 123: " << user1.value_or("Unknown") << std::endl;
    std::cout << "User 999: " << user2.value_or("Unknown") << std::endl;
    
    // Monadic operations (C++23 preview)
    std::optional<int> chain_example = 5;
    if (chain_example) {
        auto doubled = *chain_example * 2;
        std::cout << "Chained result: " << doubled << std::endl;
    }
}

// =============================================================================
// 2. STD::VARIANT - TYPE-SAFE UNIONS
// =============================================================================

void process_variant(const std::variant<int, double, std::string>& v) {
    // Using std::visit with lambda
    std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "Processing int: " << value << std::endl;
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "Processing double: " << value << std::endl;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "Processing string: " << value << std::endl;
        }
    }, v);
}

// Visitor class approach
struct VariantVisitor {
    void operator()(int i) const {
        std::cout << "Visitor got int: " << i << std::endl;
    }
    
    void operator()(double d) const {
        std::cout << "Visitor got double: " << d << std::endl;
    }
    
    void operator()(const std::string& s) const {
        std::cout << "Visitor got string: " << s << std::endl;
    }
};

void demonstrate_variant() {
    SECTION("std::variant - Type-Safe Unions");
    
    // Basic usage
    std::variant<int, double, std::string> var;
    var = 42;  // Holds int
    
    std::cout << "Variant index: " << var.index() << std::endl;  // 0 for int
    std::cout << "Holds int: " << std::holds_alternative<int>(var) << std::endl;
    
    // Access value
    if (std::holds_alternative<int>(var)) {
        std::cout << "Int value: " << std::get<int>(var) << std::endl;
    }
    
    // Change type
    var = 3.14;  // Now holds double
    std::cout << "New index: " << var.index() << std::endl;  // 1 for double
    
    var = "Hello";  // Now holds string
    std::cout << "String value: " << std::get<std::string>(var) << std::endl;
    
    // Using std::get_if for safe access
    if (auto* str_ptr = std::get_if<std::string>(&var)) {
        std::cout << "Safe string access: " << *str_ptr << std::endl;
    }
    
    // Visit pattern
    process_variant(42);
    process_variant(2.71);
    process_variant("Variant");
    
    // Visitor class
    std::visit(VariantVisitor{}, var);
    
    // Vector of variants
    std::vector<std::variant<int, double, std::string>> values{
        42, 3.14, "mixed", 100, 2.71, "types"
    };
    
    for (const auto& val : values) {
        process_variant(val);
    }
}

// =============================================================================
// 3. STD::ANY - TYPE-ERASED STORAGE
// =============================================================================

void process_any(const std::any& a) {
    std::cout << "any type: " << a.type().name() << std::endl;
    
    // Try to cast to different types
    if (auto* int_ptr = std::any_cast<int>(&a)) {
        std::cout << "any contains int: " << *int_ptr << std::endl;
    } else if (auto* double_ptr = std::any_cast<double>(&a)) {
        std::cout << "any contains double: " << *double_ptr << std::endl;
    } else if (auto* string_ptr = std::any_cast<std::string>(&a)) {
        std::cout << "any contains string: " << *string_ptr << std::endl;
    } else {
        std::cout << "any contains unknown type" << std::endl;
    }
}

void demonstrate_any() {
    SECTION("std::any - Type-Erased Storage");
    
    // Basic usage
    std::any any_val;
    std::cout << "Empty any has value: " << any_val.has_value() << std::endl;
    
    any_val = 42;
    std::cout << "any has value: " << any_val.has_value() << std::endl;
    
    // Access with any_cast
    try {
        int value = std::any_cast<int>(any_val);
        std::cout << "Int from any: " << value << std::endl;
    } catch (const std::bad_any_cast& e) {
        std::cout << "Bad cast: " << e.what() << std::endl;
    }
    
    // Change type
    any_val = 3.14;
    process_any(any_val);
    
    any_val = std::string("Hello Any");
    process_any(any_val);
    
    // Safe casting with pointer version
    if (auto* str_ptr = std::any_cast<std::string>(&any_val)) {
        std::cout << "Safe any cast: " << *str_ptr << std::endl;
    }
    
    // Vector of any
    std::vector<std::any> mixed_data{
        42, 3.14, std::string("mixed"), true, 'c'
    };
    
    for (const auto& item : mixed_data) {
        process_any(item);
    }
    
    // Reset any
    any_val.reset();
    std::cout << "After reset, has value: " << any_val.has_value() << std::endl;
}

// =============================================================================
// 4. PRACTICAL COMBINATIONS
// =============================================================================

// Configuration system using variant
using ConfigValue = std::variant<int, double, std::string, bool>;

class Configuration {
private:
    std::map<std::string, ConfigValue> config_;
    
public:
    void set(const std::string& key, const ConfigValue& value) {
        config_[key] = value;
    }
    
    std::optional<ConfigValue> get(const std::string& key) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    template<typename T>
    std::optional<T> get_as(const std::string& key) const {
        if (auto opt_val = get(key)) {
            if (std::holds_alternative<T>(*opt_val)) {
                return std::get<T>(*opt_val);
            }
        }
        return std::nullopt;
    }
};

void demonstrate_practical_combinations() {
    SECTION("Practical Combinations");
    
    Configuration config;
    config.set("port", 8080);
    config.set("debug", true);
    config.set("host", std::string("localhost"));
    config.set("timeout", 30.5);
    
    // Safe retrieval
    if (auto port = config.get_as<int>("port")) {
        std::cout << "Port: " << *port << std::endl;
    }
    
    if (auto debug = config.get_as<bool>("debug")) {
        std::cout << "Debug: " << std::boolalpha << *debug << std::endl;
    }
    
    if (auto host = config.get_as<std::string>("host")) {
        std::cout << "Host: " << *host << std::endl;
    }
    
    // Non-existent key
    if (auto missing = config.get_as<int>("missing")) {
        std::cout << "Missing value: " << *missing << std::endl;
    } else {
        std::cout << "Key 'missing' not found" << std::endl;
    }
    
    // Wrong type
    if (auto wrong_type = config.get_as<std::string>("port")) {
        std::cout << "Port as string: " << *wrong_type << std::endl;
    } else {
        std::cout << "Port is not a string" << std::endl;
    }
}

// =============================================================================
// 5. ERROR HANDLING WITH OPTIONAL
// =============================================================================

// Result type using optional
template<typename T, typename E>
class Result {
private:
    std::optional<T> value_;
    std::optional<E> error_;
    
public:
    static Result success(T value) {
        Result r;
        r.value_ = std::move(value);
        return r;
    }
    
    static Result error(E error) {
        Result r;
        r.error_ = std::move(error);
        return r;
    }
    
    bool is_success() const { return value_.has_value(); }
    bool is_error() const { return error_.has_value(); }
    
    const T& value() const { return *value_; }
    const E& error() const { return *error_; }
};

Result<int, std::string> parse_int(const std::string& str) {
    try {
        int value = std::stoi(str);
        return Result<int, std::string>::success(value);
    } catch (const std::exception&) {
        return Result<int, std::string>::error("Invalid integer: " + str);
    }
}

void demonstrate_error_handling() {
    SECTION("Error Handling with Optional and Variant");
    
    auto result1 = parse_int("123");
    auto result2 = parse_int("abc");
    
    if (result1.is_success()) {
        std::cout << "Parsed: " << result1.value() << std::endl;
    }
    
    if (result2.is_error()) {
        std::cout << "Error: " << result2.error() << std::endl;
    }
}

int main() {
    std::cout << "STD::OPTIONAL, STD::VARIANT, STD::ANY TUTORIAL\n";
    std::cout << "==============================================\n";
    
    demonstrate_optional();
    demonstrate_variant();
    demonstrate_any();
    demonstrate_practical_combinations();
    demonstrate_error_handling();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:

std::optional:
- Nullable values without pointers
- has_value(), value(), value_or()
- Implicit bool conversion
- std::nullopt for empty state

std::variant:
- Type-safe union of types
- std::holds_alternative<T>(), std::get<T>(), std::get_if<T>()
- std::visit() for pattern matching
- index() to get current type index

std::any:
- Type-erased storage for any type
- std::any_cast<T>() for retrieval
- type() for runtime type info
- Throws std::bad_any_cast on wrong cast

All three are C++17 features providing safer alternatives to raw pointers,
unions, and void* respectively.
*/
