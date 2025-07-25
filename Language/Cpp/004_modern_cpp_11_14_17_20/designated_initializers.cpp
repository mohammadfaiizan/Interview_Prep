/*
 * MODERN C++ - DESIGNATED INITIALIZERS (C++20)
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra designated_initializers.cpp -o designated_initializers
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC DESIGNATED INITIALIZATION
// =============================================================================

struct Point {
    int x;
    int y;
    int z = 0;  // Default value
};

struct Person {
    std::string name;
    int age;
    std::string city = "Unknown";
};

void demonstrate_basic_designated_init() {
    SECTION("Basic Designated Initialization");
    
    // Traditional initialization
    Point p1{10, 20, 30};
    std::cout << "Traditional: (" << p1.x << ", " << p1.y << ", " << p1.z << ")" << std::endl;
    
    // Designated initialization (C++20)
    Point p2{.x = 100, .y = 200, .z = 300};
    std::cout << "Designated: (" << p2.x << ", " << p2.y << ", " << p2.z << ")" << std::endl;
    
    // Partial initialization with defaults
    Point p3{.x = 5, .y = 10};  // z uses default value 0
    std::cout << "Partial: (" << p3.x << ", " << p3.y << ", " << p3.z << ")" << std::endl;
    
    // Order matters - must match declaration order
    Point p4{.x = 1, .y = 2};  // OK
    // Point p5{.y = 2, .x = 1};  // Error: wrong order
    
    // With strings
    Person person1{.name = "Alice", .age = 30, .city = "New York"};
    std::cout << "Person: " << person1.name << ", " << person1.age 
              << ", " << person1.city << std::endl;
    
    // Skip middle fields
    Person person2{.name = "Bob", .city = "London"};  // age gets zero-initialized
    std::cout << "Person2: " << person2.name << ", " << person2.age 
              << ", " << person2.city << std::endl;
}

// =============================================================================
// 2. DESIGNATED INITIALIZATION WITH NESTED STRUCTS
// =============================================================================

struct Address {
    std::string street;
    std::string city;
    int zip_code;
};

struct Employee {
    std::string name;
    int id;
    Address address;
    double salary = 0.0;
};

void demonstrate_nested_designated_init() {
    SECTION("Nested Designated Initialization");
    
    // Nested designated initialization
    Employee emp{
        .name = "John Doe",
        .id = 12345,
        .address = {
            .street = "123 Main St",
            .city = "Springfield",
            .zip_code = 12345
        },
        .salary = 75000.0
    };
    
    std::cout << "Employee: " << emp.name << " (ID: " << emp.id << ")" << std::endl;
    std::cout << "Address: " << emp.address.street << ", " 
              << emp.address.city << " " << emp.address.zip_code << std::endl;
    std::cout << "Salary: $" << emp.salary << std::endl;
    
    // Can also initialize nested struct separately
    Address addr{.street = "456 Oak Ave", .city = "Portland", .zip_code = 97201};
    Employee emp2{.name = "Jane Smith", .id = 67890, .address = addr};
    
    std::cout << "\nEmployee2: " << emp2.name << std::endl;
    std::cout << "Lives at: " << emp2.address.street << std::endl;
}

// =============================================================================
// 3. DESIGNATED INITIALIZATION WITH ARRAYS
// =============================================================================

struct Matrix {
    int data[3][3];
    std::string name;
};

struct Config {
    std::array<int, 4> ports;
    bool debug_mode;
    std::string log_file;
};

void demonstrate_array_designated_init() {
    SECTION("Arrays with Designated Initialization");
    
    // Arrays in structs
    Matrix identity{
        .data = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
        .name = "Identity Matrix"
    };
    
    std::cout << identity.name << ":" << std::endl;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::cout << identity.data[i][j] << " ";
        }
        std::cout << std::endl;
    }
    
    // std::array with designated init
    Config config{
        .ports = {8080, 8443, 9000, 9001},
        .debug_mode = true,
        .log_file = "app.log"
    };
    
    std::cout << "\nConfig ports: ";
    for (int port : config.ports) {
        std::cout << port << " ";
    }
    std::cout << std::endl;
    std::cout << "Debug: " << std::boolalpha << config.debug_mode << std::endl;
}

// =============================================================================
// 4. COMPARISON WITH TRADITIONAL INITIALIZATION
// =============================================================================

struct Rectangle {
    double width;
    double height;
    std::string color = "white";
    bool filled = false;
};

void demonstrate_comparison() {
    SECTION("Comparison: Traditional vs Designated");
    
    // Traditional positional initialization
    Rectangle rect1{10.0, 5.0, "red", true};
    
    // Designated initialization - much clearer!
    Rectangle rect2{
        .width = 10.0,
        .height = 5.0,
        .color = "red",
        .filled = true
    };
    
    // Partial initialization with designated
    Rectangle rect3{.width = 15.0, .height = 8.0};  // color and filled use defaults
    
    // Easy to see what's being set
    Rectangle rect4{.color = "blue", .filled = true};  // width and height zero-initialized
    
    std::cout << "rect1: " << rect1.width << "x" << rect1.height 
              << ", " << rect1.color << ", filled=" << std::boolalpha << rect1.filled << std::endl;
    
    std::cout << "rect3: " << rect3.width << "x" << rect3.height 
              << ", " << rect3.color << ", filled=" << std::boolalpha << rect3.filled << std::endl;
    
    std::cout << "rect4: " << rect4.width << "x" << rect4.height 
              << ", " << rect4.color << ", filled=" << std::boolalpha << rect4.filled << std::endl;
}

// =============================================================================
// 5. DESIGNATED INITIALIZATION WITH INHERITANCE
// =============================================================================

struct Base {
    int base_value;
    std::string base_name;
};

struct Derived : Base {
    int derived_value;
    double derived_ratio;
};

void demonstrate_inheritance() {
    SECTION("Designated Initialization with Inheritance");
    
    // Note: Designated initialization doesn't work directly with inheritance
    // You need to initialize the base class part first
    
    // This works but is not as clean:
    Derived d1{{.base_value = 100, .base_name = "base"}, .derived_value = 200, .derived_ratio = 1.5};
    
    std::cout << "Derived object:" << std::endl;
    std::cout << "Base: " << d1.base_value << ", " << d1.base_name << std::endl;
    std::cout << "Derived: " << d1.derived_value << ", " << d1.derived_ratio << std::endl;
    
    // Alternative: use aggregate initialization
    Derived d2{100, "base2", 300, 2.5};
    std::cout << "\nAlternative:" << std::endl;
    std::cout << "Base: " << d2.base_value << ", " << d2.base_name << std::endl;
    std::cout << "Derived: " << d2.derived_value << ", " << d2.derived_ratio << std::endl;
}

// =============================================================================
// 6. REAL-WORLD EXAMPLES
// =============================================================================

struct DatabaseConfig {
    std::string host = "localhost";
    int port = 5432;
    std::string database;
    std::string username;
    std::string password;
    int connection_timeout = 30;
    int max_connections = 10;
    bool ssl_enabled = false;
};

struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "Application";
    bool resizable = true;
    bool fullscreen = false;
    int x_position = -1;  // -1 means center
    int y_position = -1;
};

void demonstrate_real_world_examples() {
    SECTION("Real-World Examples");
    
    // Database configuration with sensible defaults
    DatabaseConfig db_config{
        .database = "myapp",
        .username = "admin",
        .password = "secret123",
        .ssl_enabled = true
    };
    // host, port, timeouts use defaults
    
    std::cout << "Database Config:" << std::endl;
    std::cout << "  Host: " << db_config.host << ":" << db_config.port << std::endl;
    std::cout << "  Database: " << db_config.database << std::endl;
    std::cout << "  SSL: " << std::boolalpha << db_config.ssl_enabled << std::endl;
    
    // Window configuration
    WindowConfig window{
        .width = 1200,
        .height = 800,
        .title = "My Game",
        .fullscreen = true
    };
    // Other settings use defaults
    
    std::cout << "\nWindow Config:" << std::endl;
    std::cout << "  Size: " << window.width << "x" << window.height << std::endl;
    std::cout << "  Title: " << window.title << std::endl;
    std::cout << "  Fullscreen: " << std::boolalpha << window.fullscreen << std::endl;
    
    // Easy to create variations
    WindowConfig debug_window{
        .width = 600,
        .height = 400,
        .title = "Debug Console",
        .resizable = false
    };
    
    std::cout << "\nDebug Window: " << debug_window.title 
              << " (" << debug_window.width << "x" << debug_window.height << ")" << std::endl;
}

// =============================================================================
// 7. LIMITATIONS AND BEST PRACTICES
// =============================================================================

void demonstrate_limitations() {
    SECTION("Limitations and Best Practices");
    
    std::cout << "Designated Initializer Limitations:" << std::endl;
    std::cout << "1. Order must match declaration order" << std::endl;
    std::cout << "2. Cannot mix with positional initialization" << std::endl;
    std::cout << "3. Only works with aggregate types" << std::endl;
    std::cout << "4. Limited support with inheritance" << std::endl;
    
    std::cout << "\nBest Practices:" << std::endl;
    std::cout << "1. Use for structs with many fields" << std::endl;
    std::cout << "2. Provide sensible defaults" << std::endl;
    std::cout << "3. Order fields logically in declaration" << std::endl;
    std::cout << "4. Consider when readability is important" << std::endl;
    
    // Example of good practice: optional parameters pattern
    struct ApiRequest {
        std::string endpoint;          // Required
        std::string method = "GET";    // Optional with default
        int timeout = 30;             // Optional with default
        bool verify_ssl = true;       // Optional with default
        std::string user_agent = "MyApp/1.0";  // Optional with default
    };
    
    // Very clear what's being set
    ApiRequest request{
        .endpoint = "/api/users",
        .method = "POST",
        .timeout = 60
    };
    
    std::cout << "\nAPI Request:" << std::endl;
    std::cout << "  " << request.method << " " << request.endpoint << std::endl;
    std::cout << "  Timeout: " << request.timeout << "s" << std::endl;
}

int main() {
    std::cout << "DESIGNATED INITIALIZERS (C++20) TUTORIAL\n";
    std::cout << "========================================\n";
    
    demonstrate_basic_designated_init();
    demonstrate_nested_designated_init();
    demonstrate_array_designated_init();
    demonstrate_comparison();
    demonstrate_inheritance();
    demonstrate_real_world_examples();
    demonstrate_limitations();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Designated initializers: struct_name{.field1 = value1, .field2 = value2}
- Must maintain declaration order
- Can skip fields (they get default/zero initialized)
- Works with nested structs
- Improves code readability and maintainability
- Only works with aggregate types (no constructors, private members, etc.)
- C++20 feature - requires modern compiler
- Great for configuration structs and optional parameters pattern
*/
