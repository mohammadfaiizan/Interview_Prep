/*
 * MODERN C++ - MOVE SEMANTICS AND RVALUE REFERENCES
 * Comprehensive guide to move semantics (C++11+)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra move_semantics_rvalue.cpp -o move_semantics
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <chrono>
#include <algorithm>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl
#define DEBUG_PRINT(x) std::cout << #x << " = " << (x) << std::endl

// =============================================================================
// 1. UNDERSTANDING LVALUES AND RVALUES
// =============================================================================

void demonstrate_lvalue_rvalue_basics() {
    SECTION("Lvalue vs Rvalue Basics");
    
    int x = 42;          // x is lvalue, 42 is rvalue
    int y = x;           // x is lvalue (has name and address)
    int z = x + y;       // x+y is rvalue (temporary expression result)
    
    // lvalue reference (C++98)
    int& lref = x;       // OK: lvalue reference to lvalue
    // int& bad_ref = 42;   // Error: cannot bind lvalue ref to rvalue
    
    // rvalue reference (C++11)
    int&& rref = 42;     // OK: rvalue reference to rvalue
    int&& rref2 = x + y; // OK: rvalue reference to rvalue expression
    // int&& bad_rref = x;  // Error: cannot bind rvalue ref to lvalue
    
    // const lvalue reference can bind to rvalues (lifetime extension)
    const int& const_ref = 42; // OK: const lref extends rvalue lifetime
    
    std::cout << "x = " << x << ", lref = " << lref << std::endl;
    std::cout << "rref = " << rref << ", rref2 = " << rref2 << std::endl;
    std::cout << "const_ref = " << const_ref << std::endl;
}

// =============================================================================
// 2. MOVE CONSTRUCTOR AND MOVE ASSIGNMENT
// =============================================================================

class MoveableString {
private:
    char* data;
    size_t size;
    
public:
    // Default constructor
    MoveableString() : data(nullptr), size(0) {
        std::cout << "Default constructor" << std::endl;
    }
    
    // Constructor with C-string
    MoveableString(const char* str) {
        size = strlen(str);
        data = new char[size + 1];
        strcpy(data, str);
        std::cout << "Constructor: " << data << std::endl;
    }
    
    // Copy constructor
    MoveableString(const MoveableString& other) {
        size = other.size;
        data = new char[size + 1];
        strcpy(data, other.data);
        std::cout << "Copy constructor: " << data << std::endl;
    }
    
    // Move constructor (C++11)
    MoveableString(MoveableString&& other) noexcept {
        data = other.data;
        size = other.size;
        other.data = nullptr;  // Leave other in valid but unspecified state
        other.size = 0;
        std::cout << "Move constructor: " << (data ? data : "null") << std::endl;
    }
    
    // Copy assignment operator
    MoveableString& operator=(const MoveableString& other) {
        std::cout << "Copy assignment" << std::endl;
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = new char[size + 1];
            strcpy(data, other.data);
        }
        return *this;
    }
    
    // Move assignment operator (C++11)
    MoveableString& operator=(MoveableString&& other) noexcept {
        std::cout << "Move assignment" << std::endl;
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
    
    // Destructor
    ~MoveableString() {
        std::cout << "Destructor: " << (data ? data : "null") << std::endl;
        delete[] data;
    }
    
    // Utility methods
    const char* c_str() const { return data ? data : ""; }
    size_t length() const { return size; }
    
    // Friend function for output
    friend std::ostream& operator<<(std::ostream& os, const MoveableString& str) {
        return os << str.c_str();
    }
};

void demonstrate_move_semantics() {
    SECTION("Move Constructor and Move Assignment");
    
    // Copy semantics
    std::cout << "\n--- Copy Semantics ---" << std::endl;
    MoveableString str1("Hello");
    MoveableString str2 = str1;  // Copy constructor
    MoveableString str3;
    str3 = str1;                 // Copy assignment
    
    // Move semantics
    std::cout << "\n--- Move Semantics ---" << std::endl;
    MoveableString str4 = std::move(str1);  // Move constructor
    MoveableString str5;
    str5 = std::move(str2);                 // Move assignment
    
    // Return value optimization (RVO) and move
    auto create_string = []() { return MoveableString("Temporary"); };
    MoveableString str6 = create_string();  // RVO or move
    
    std::cout << "\nFinal values:" << std::endl;
    std::cout << "str1: '" << str1 << "'" << std::endl;
    std::cout << "str2: '" << str2 << "'" << std::endl;
    std::cout << "str3: '" << str3 << "'" << std::endl;
    std::cout << "str4: '" << str4 << "'" << std::endl;
    std::cout << "str5: '" << str5 << "'" << std::endl;
    std::cout << "str6: '" << str6 << "'" << std::endl;
}

// =============================================================================
// 3. STD::MOVE AND MOVE SEMANTICS WITH STL
// =============================================================================

void demonstrate_stl_move_semantics() {
    SECTION("Move Semantics with STL");
    
    // std::vector and move semantics
    std::vector<MoveableString> vec;
    
    std::cout << "\n--- push_back with copy ---" << std::endl;
    MoveableString str("Copy Me");
    vec.push_back(str);  // Copy
    
    std::cout << "\n--- push_back with move ---" << std::endl;
    vec.push_back(std::move(str));  // Move
    
    std::cout << "\n--- emplace_back ---" << std::endl;
    vec.emplace_back("Direct Construction");  // Direct construction
    
    std::cout << "\n--- Moving entire vector ---" << std::endl;
    std::vector<MoveableString> vec2 = std::move(vec);  // Move entire vector
    
    std::cout << "Original vector size: " << vec.size() << std::endl;
    std::cout << "Moved vector size: " << vec2.size() << std::endl;
}

// =============================================================================
// 4. PERFECT FORWARDING
// =============================================================================

template<typename T>
void process_value(T&& arg) {
    std::cout << "Processing: " << arg << std::endl;
}

template<typename T>
void forward_to_process(T&& arg) {
    // Without perfect forwarding - always forwards as lvalue
    // process_value(arg);
    
    // With perfect forwarding - preserves value category
    process_value(std::forward<T>(arg));
}

void demonstrate_perfect_forwarding() {
    SECTION("Perfect Forwarding");
    
    MoveableString str("Forward Me");
    
    std::cout << "\n--- Forwarding lvalue ---" << std::endl;
    forward_to_process(str);
    
    std::cout << "\n--- Forwarding rvalue ---" << std::endl;
    forward_to_process(MoveableString("Temporary"));
    
    std::cout << "\n--- Forwarding with std::move ---" << std::endl;
    forward_to_process(std::move(str));
}

// =============================================================================
// 5. UNIVERSAL REFERENCES AND REFERENCE COLLAPSING
// =============================================================================

template<typename T>
void universal_reference_demo(T&& param) {
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
    SECTION("Universal References and Reference Collapsing");
    
    int x = 42;
    
    std::cout << "\n--- Passing lvalue ---" << std::endl;
    universal_reference_demo(x);  // T deduced as int&, T&& becomes int&
    
    std::cout << "\n--- Passing rvalue ---" << std::endl;
    universal_reference_demo(42); // T deduced as int, T&& becomes int&&
}

// =============================================================================
// 6. MOVE-ONLY TYPES
// =============================================================================

class MoveOnlyResource {
private:
    std::unique_ptr<int> resource;
    std::string name;
    
public:
    MoveOnlyResource(const std::string& n, int value) 
        : resource(std::make_unique<int>(value)), name(n) {
        std::cout << "Creating " << name << " with value " << value << std::endl;
    }
    
    // Delete copy constructor and copy assignment
    MoveOnlyResource(const MoveOnlyResource&) = delete;
    MoveOnlyResource& operator=(const MoveOnlyResource&) = delete;
    
    // Default move constructor and move assignment
    MoveOnlyResource(MoveOnlyResource&&) = default;
    MoveOnlyResource& operator=(MoveOnlyResource&&) = default;
    
    ~MoveOnlyResource() {
        std::cout << "Destroying " << name << std::endl;
    }
    
    int getValue() const { return resource ? *resource : 0; }
    const std::string& getName() const { return name; }
};

void demonstrate_move_only_types() {
    SECTION("Move-Only Types");
    
    std::vector<MoveOnlyResource> resources;
    
    // Must use move or emplace_back
    resources.emplace_back("Resource1", 100);
    resources.push_back(MoveOnlyResource("Resource2", 200));
    
    MoveOnlyResource temp("Resource3", 300);
    resources.push_back(std::move(temp));
    
    // Function that returns move-only type
    auto create_resource = [](const std::string& name, int value) {
        return MoveOnlyResource(name, value);
    };
    
    auto resource = create_resource("Created", 400);
    
    for (const auto& r : resources) {
        std::cout << r.getName() << ": " << r.getValue() << std::endl;
    }
}

// =============================================================================
// 7. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison: Copy vs Move");
    
    const int iterations = 10000;
    
    // Test with vector of strings
    auto time_copy = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::string> source;
        for (int i = 0; i < iterations; ++i) {
            source.push_back("String " + std::to_string(i));
        }
        
        std::vector<std::string> dest;
        for (const auto& str : source) {
            dest.push_back(str);  // Copy
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };
    
    auto time_move = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::string> source;
        for (int i = 0; i < iterations; ++i) {
            source.push_back("String " + std::to_string(i));
        }
        
        std::vector<std::string> dest;
        for (auto& str : source) {
            dest.push_back(std::move(str));  // Move
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };
    
    double copy_time = time_copy();
    double move_time = time_move();
    
    std::cout << "Copy time: " << copy_time << " ms" << std::endl;
    std::cout << "Move time: " << move_time << " ms" << std::endl;
    std::cout << "Speedup: " << (copy_time / move_time) << "x" << std::endl;
}

int main() {
    std::cout << "MOVE SEMANTICS AND RVALUE REFERENCES TUTORIAL\n";
    std::cout << "============================================\n";
    
    demonstrate_lvalue_rvalue_basics();
    demonstrate_move_semantics();
    demonstrate_stl_move_semantics();
    demonstrate_perfect_forwarding();
    demonstrate_universal_references();
    demonstrate_move_only_types();
    performance_comparison();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- lvalue: has name and address, rvalue: temporary value
- Move constructor: transfers ownership, leaves source in valid state
- Move assignment: same as move constructor but for assignment
- std::move: converts lvalue to rvalue reference
- Perfect forwarding: preserves value category in templates
- Universal references: T&& in template context
- Reference collapsing: T& && becomes T&
- Move-only types: delete copy, default move operations
- Significant performance improvements for expensive-to-copy types
*/
