/*
 * C++ LATEST AND FUTURE - C++20 MODULES
 * 
 * Comprehensive guide to C++20 modules system including module interfaces,
 * implementations, partitions, and migration strategies.
 * 
 * Compilation: 
 * g++ -std=c++20 -fmodules-ts -c math_module.cppm
 * g++ -std=c++20 -fmodules-ts cpp_modules_cpp20.cpp math_module.o -o modules_demo
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <concepts>

// Note: In real C++20 modules, we would use 'import' statements
// For demonstration, we'll show the module syntax in comments and simulate

/*
// This would be in a separate file: math_module.cppm
export module math;

export namespace math {
    double add(double a, double b);
    double multiply(double a, double b);
    
    template<typename T>
    T power(T base, int exp);
    
    class Calculator;
}

// Implementation would be in math_module.cpp or same file
module math;

namespace math {
    double add(double a, double b) {
        return a + b;
    }
    
    double multiply(double a, double b) {
        return a * b;
    }
    
    template<typename T>
    T power(T base, int exp) {
        T result = 1;
        for (int i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }
    
    export class Calculator {
    public:
        double calculate(const std::string& operation, double a, double b);
    };
}
*/

// =============================================================================
// SIMULATED MODULE SYSTEM (for demonstration)
// =============================================================================

// Simulate module interface
namespace math_module {
    // Exported functions
    double add(double a, double b) {
        std::cout << "Module math::add(" << a << ", " << b << ") = ";
        double result = a + b;
        std::cout << result << std::endl;
        return result;
    }
    
    double multiply(double a, double b) {
        std::cout << "Module math::multiply(" << a << ", " << b << ") = ";
        double result = a * b;
        std::cout << result << std::endl;
        return result;
    }
    
    template<typename T>
    T power(T base, int exp) {
        std::cout << "Module math::power(" << base << ", " << exp << ") = ";
        T result = 1;
        for (int i = 0; i < exp; ++i) {
            result *= base;
        }
        std::cout << result << std::endl;
        return result;
    }
    
    // Exported class
    class Calculator {
    public:
        double calculate(const std::string& operation, double a, double b) {
            std::cout << "Calculator::" << operation << "(" << a << ", " << b << ") = ";
            
            if (operation == "add") {
                return add(a, b);
            } else if (operation == "multiply") {
                return multiply(a, b);
            } else {
                std::cout << "Unknown operation" << std::endl;
                return 0.0;
            }
        }
        
        template<typename T>
        T generic_operation(T a, T b, auto op) {
            return op(a, b);
        }
    };
    
    // Non-exported (private) function
    namespace internal {
        void debug_log(const std::string& message) {
            std::cout << "[DEBUG] " << message << std::endl;
        }
    }
}

// Simulate string utilities module
namespace string_utils_module {
    class StringProcessor {
    public:
        static std::string to_upper(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            std::cout << "StringProcessor::to_upper(\"" << str << "\") = \"" << result << "\"" << std::endl;
            return result;
        }
        
        static std::string reverse(const std::string& str) {
            std::string result(str.rbegin(), str.rend());
            std::cout << "StringProcessor::reverse(\"" << str << "\") = \"" << result << "\"" << std::endl;
            return result;
        }
        
        static std::vector<std::string> split(const std::string& str, char delimiter) {
            std::vector<std::string> tokens;
            std::stringstream ss(str);
            std::string token;
            
            while (std::getline(ss, token, delimiter)) {
                tokens.push_back(token);
            }
            
            std::cout << "StringProcessor::split(\"" << str << "\", '" << delimiter 
                      << "') = " << tokens.size() << " tokens" << std::endl;
            return tokens;
        }
    };
    
    // Module-level function
    std::string format_message(const std::string& template_str, const std::vector<std::string>& args) {
        std::string result = template_str;
        for (size_t i = 0; i < args.size(); ++i) {
            std::string placeholder = "{" + std::to_string(i) + "}";
            size_t pos = result.find(placeholder);
            if (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
            }
        }
        
        std::cout << "format_message result: \"" << result << "\"" << std::endl;
        return result;
    }
}

// =============================================================================
// MODULE PARTITIONS DEMONSTRATION
// =============================================================================

// Simulate module partitions
namespace graphics_module {
    // Partition: graphics:shapes
    namespace shapes {
        class Circle {
        private:
            double radius_;
            
        public:
            Circle(double r) : radius_(r) {}
            
            double area() const {
                double result = 3.14159 * radius_ * radius_;
                std::cout << "Circle::area() with radius " << radius_ << " = " << result << std::endl;
                return result;
            }
            
            double circumference() const {
                double result = 2 * 3.14159 * radius_;
                std::cout << "Circle::circumference() with radius " << radius_ << " = " << result << std::endl;
                return result;
            }
        };
        
        class Rectangle {
        private:
            double width_, height_;
            
        public:
            Rectangle(double w, double h) : width_(w), height_(h) {}
            
            double area() const {
                double result = width_ * height_;
                std::cout << "Rectangle::area() " << width_ << "x" << height_ << " = " << result << std::endl;
                return result;
            }
            
            double perimeter() const {
                double result = 2 * (width_ + height_);
                std::cout << "Rectangle::perimeter() " << width_ << "x" << height_ << " = " << result << std::endl;
                return result;
            }
        };
    }
    
    // Partition: graphics:colors
    namespace colors {
        struct Color {
            uint8_t r, g, b, a;
            
            Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
                : r(red), g(green), b(blue), a(alpha) {}
            
            std::string to_hex() const {
                char hex[10];
                sprintf(hex, "#%02X%02X%02X", r, g, b);
                std::string result(hex);
                std::cout << "Color::to_hex() RGB(" << (int)r << "," << (int)g << "," << (int)b << ") = " << result << std::endl;
                return result;
            }
        };
        
        // Predefined colors
        const Color RED{255, 0, 0};
        const Color GREEN{0, 255, 0};
        const Color BLUE{0, 0, 255};
        const Color WHITE{255, 255, 255};
        const Color BLACK{0, 0, 0};
    }
    
    // Main module interface combining partitions
    class Renderer {
    public:
        void render_shape(const shapes::Circle& circle, const colors::Color& color) {
            std::cout << "Rendering Circle with area " << circle.area() 
                      << " in color " << color.to_hex() << std::endl;
        }
        
        void render_shape(const shapes::Rectangle& rect, const colors::Color& color) {
            std::cout << "Rendering Rectangle with area " << rect.area() 
                      << " in color " << color.to_hex() << std::endl;
        }
    };
}

// =============================================================================
// MODULE INTERFACE DESIGN PATTERNS
// =============================================================================

namespace design_patterns_module {
    // Factory pattern in modules
    template<typename T>
    class Factory {
    public:
        template<typename... Args>
        static std::unique_ptr<T> create(Args&&... args) {
            std::cout << "Factory creating object of type: " << typeid(T).name() << std::endl;
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
    };
    
    // Observer pattern
    template<typename EventType>
    class EventBus {
    private:
        std::vector<std::function<void(const EventType&)>> listeners_;
        
    public:
        void subscribe(std::function<void(const EventType&)> listener) {
            listeners_.push_back(std::move(listener));
            std::cout << "EventBus: Subscribed listener (total: " << listeners_.size() << ")" << std::endl;
        }
        
        void publish(const EventType& event) {
            std::cout << "EventBus: Publishing event to " << listeners_.size() << " listeners" << std::endl;
            for (auto& listener : listeners_) {
                listener(event);
            }
        }
    };
    
    // Example event type
    struct MessageEvent {
        std::string message;
        int priority;
        
        MessageEvent(const std::string& msg, int prio = 0) : message(msg), priority(prio) {}
    };
}

// =============================================================================
// MODULE CONCEPTS AND CONSTRAINTS
// =============================================================================

namespace concepts_module {
    // Concept definitions for module interfaces
    template<typename T>
    concept Drawable = requires(T t) {
        { t.draw() } -> std::same_as<void>;
        { t.get_bounds() } -> std::convertible_to<std::string>;
    };
    
    template<typename T>
    concept Serializable = requires(T t) {
        { t.serialize() } -> std::convertible_to<std::string>;
        { T::deserialize(std::string{}) } -> std::same_as<T>;
    };
    
    template<typename T>
    concept Comparable = requires(T a, T b) {
        { a < b } -> std::convertible_to<bool>;
        { a == b } -> std::convertible_to<bool>;
    };
    
    // Generic algorithms using concepts
    template<Drawable T>
    void render_object(const T& obj) {
        std::cout << "Rendering object with bounds: " << obj.get_bounds() << std::endl;
        obj.draw();
    }
    
    template<Serializable T>
    void save_object(const T& obj, const std::string& filename) {
        std::string data = obj.serialize();
        std::cout << "Saving object to " << filename << " (size: " << data.length() << " bytes)" << std::endl;
    }
    
    template<Comparable T>
    std::vector<T> sort_objects(std::vector<T> objects) {
        std::sort(objects.begin(), objects.end());
        std::cout << "Sorted " << objects.size() << " objects" << std::endl;
        return objects;
    }
    
    // Example classes that satisfy concepts
    class DrawableShape {
    public:
        void draw() const {
            std::cout << "Drawing shape..." << std::endl;
        }
        
        std::string get_bounds() const {
            return "bounds(0,0,100,100)";
        }
    };
    
    class SerializableData {
    private:
        std::string data_;
        
    public:
        SerializableData(const std::string& data = "") : data_(data) {}
        
        std::string serialize() const {
            return "serialized:" + data_;
        }
        
        static SerializableData deserialize(const std::string& str) {
            if (str.starts_with("serialized:")) {
                return SerializableData(str.substr(11));
            }
            return SerializableData();
        }
    };
}

// =============================================================================
// MODULE MIGRATION STRATEGIES
// =============================================================================

namespace migration_demo {
    // Legacy header-based code
    namespace legacy {
        class LegacyCalculator {
        public:
            static double add(double a, double b) {
                std::cout << "Legacy::add(" << a << ", " << b << ") = " << (a + b) << std::endl;
                return a + b;
            }
            
            static double subtract(double a, double b) {
                std::cout << "Legacy::subtract(" << a << ", " << b << ") = " << (a - b) << std::endl;
                return a - b;
            }
        };
    }
    
    // Modern module-based wrapper
    namespace modern {
        class ModernCalculator {
        public:
            // Wrapper around legacy functionality
            static double add(double a, double b) {
                std::cout << "Modern wrapper calling legacy implementation" << std::endl;
                return legacy::LegacyCalculator::add(a, b);
            }
            
            static double subtract(double a, double b) {
                std::cout << "Modern wrapper calling legacy implementation" << std::endl;
                return legacy::LegacyCalculator::subtract(a, b);
            }
            
            // New module-specific functionality
            template<typename T>
            static T safe_divide(T a, T b) {
                if (b == T{}) {
                    std::cout << "Modern::safe_divide: Division by zero!" << std::endl;
                    return T{};
                }
                T result = a / b;
                std::cout << "Modern::safe_divide(" << a << ", " << b << ") = " << result << std::endl;
                return result;
            }
        };
    }
    
    void demonstrate_migration() {
        std::cout << "\n=== Module Migration Demo ===" << std::endl;
        
        // Using legacy code
        legacy::LegacyCalculator::add(10, 5);
        
        // Using modern wrapper
        modern::ModernCalculator::add(10, 5);
        modern::ModernCalculator::safe_divide(10.0, 3.0);
        modern::ModernCalculator::safe_divide(10, 0);
    }
}

// =============================================================================
// MODULE PERFORMANCE AND OPTIMIZATION
// =============================================================================

namespace performance_demo {
    // Template heavy computation (benefits from modules)
    template<typename T, size_t N>
    class Matrix {
    private:
        std::array<std::array<T, N>, N> data_;
        
    public:
        Matrix() {
            for (auto& row : data_) {
                row.fill(T{});
            }
        }
        
        Matrix(std::initializer_list<std::initializer_list<T>> init) : Matrix() {
            size_t i = 0;
            for (const auto& row : init) {
                if (i >= N) break;
                size_t j = 0;
                for (const auto& val : row) {
                    if (j >= N) break;
                    data_[i][j] = val;
                    ++j;
                }
                ++i;
            }
        }
        
        T& operator()(size_t row, size_t col) {
            return data_[row][col];
        }
        
        const T& operator()(size_t row, size_t col) const {
            return data_[row][col];
        }
        
        Matrix operator*(const Matrix& other) const {
            Matrix result;
            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    for (size_t k = 0; k < N; ++k) {
                        result(i, j) += data_[i][k] * other(k, j);
                    }
                }
            }
            return result;
        }
        
        void print() const {
            std::cout << "Matrix " << N << "x" << N << ":" << std::endl;
            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    std::cout << data_[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
    };
    
    void benchmark_modules() {
        std::cout << "\n=== Module Performance Demo ===" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Create and multiply matrices
        Matrix<double, 3> m1{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        Matrix<double, 3> m2{{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
        
        auto result = m1 * m2;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Matrix multiplication completed in " << duration.count() << " microseconds" << std::endl;
        result.print();
        
        std::cout << "\nModule Benefits:" << std::endl;
        std::cout << "• Faster compilation due to pre-compiled module interfaces" << std::endl;
        std::cout << "• Better optimization opportunities" << std::endl;
        std::cout << "• Reduced template instantiation overhead" << std::endl;
    }
}

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_basic_modules() {
    std::cout << "\n=== Basic Module Usage ===" << std::endl;
    
    // Using math module
    math_module::Calculator calc;
    calc.calculate("add", 10, 5);
    calc.calculate("multiply", 3, 7);
    
    // Using template functions
    math_module::power(2, 8);
    math_module::power(1.5, 3);
    
    // Generic operations
    auto result = calc.generic_operation(10, 20, [](auto a, auto b) { return a + b; });
    std::cout << "Generic operation result: " << result << std::endl;
}

void demonstrate_string_utils() {
    std::cout << "\n=== String Utils Module ===" << std::endl;
    
    string_utils_module::StringProcessor::to_upper("hello world");
    string_utils_module::StringProcessor::reverse("modules");
    
    auto tokens = string_utils_module::StringProcessor::split("one,two,three", ',');
    for (const auto& token : tokens) {
        std::cout << "Token: " << token << std::endl;
    }
    
    string_utils_module::format_message("Hello {0}, welcome to {1}!", {"Alice", "C++20 Modules"});
}

void demonstrate_module_partitions() {
    std::cout << "\n=== Module Partitions Demo ===" << std::endl;
    
    // Using shapes partition
    graphics_module::shapes::Circle circle(5.0);
    graphics_module::shapes::Rectangle rect(10.0, 8.0);
    
    // Using colors partition
    graphics_module::colors::Color red = graphics_module::colors::RED;
    graphics_module::colors::Color custom{128, 64, 192};
    
    // Using main module interface
    graphics_module::Renderer renderer;
    renderer.render_shape(circle, red);
    renderer.render_shape(rect, custom);
}

void demonstrate_design_patterns() {
    std::cout << "\n=== Design Patterns in Modules ===" << std::endl;
    
    // Factory pattern
    auto calc = design_patterns_module::Factory<math_module::Calculator>::create();
    calc->calculate("add", 15, 25);
    
    // Observer pattern
    design_patterns_module::EventBus<design_patterns_module::MessageEvent> bus;
    
    bus.subscribe([](const design_patterns_module::MessageEvent& event) {
        std::cout << "Listener 1: " << event.message << " (priority: " << event.priority << ")" << std::endl;
    });
    
    bus.subscribe([](const design_patterns_module::MessageEvent& event) {
        std::cout << "Listener 2: Processing high priority message: " << event.message << std::endl;
    });
    
    bus.publish(design_patterns_module::MessageEvent("Module system initialized", 1));
}

void demonstrate_concepts() {
    std::cout << "\n=== Concepts in Modules ===" << std::endl;
    
    // Using concepts with module types
    concepts_module::DrawableShape shape;
    concepts_module::render_object(shape);
    
    concepts_module::SerializableData data("module_data");
    concepts_module::save_object(data, "data.bin");
    
    std::vector<int> numbers{5, 2, 8, 1, 9};
    auto sorted = concepts_module::sort_objects(numbers);
    
    std::cout << "Sorted numbers: ";
    for (int n : sorted) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

void demonstrate_module_benefits() {
    std::cout << "\n=== Module System Benefits ===" << std::endl;
    
    std::cout << "1. Compilation Performance:" << std::endl;
    std::cout << "   • Faster builds due to pre-compiled interfaces" << std::endl;
    std::cout << "   • Reduced header parsing overhead" << std::endl;
    std::cout << "   • Better incremental compilation" << std::endl;
    
    std::cout << "\n2. Encapsulation:" << std::endl;
    std::cout << "   • True module boundaries" << std::endl;
    std::cout << "   • No macro pollution" << std::endl;
    std::cout << "   • Explicit export control" << std::endl;
    
    std::cout << "\n3. Scalability:" << std::endl;
    std::cout << "   • Better dependency management" << std::endl;
    std::cout << "   • Reduced template instantiation bloat" << std::endl;
    std::cout << "   • Improved link times" << std::endl;
    
    std::cout << "\n4. Tooling Support:" << std::endl;
    std::cout << "   • Better IDE integration" << std::endl;
    std::cout << "   • Enhanced debugging experience" << std::endl;
    std::cout << "   • Improved static analysis" << std::endl;
}

int main() {
    std::cout << "C++20 MODULES SYSTEM\n";
    std::cout << "====================\n";
    
    try {
        demonstrate_basic_modules();
        demonstrate_string_utils();
        demonstrate_module_partitions();
        demonstrate_design_patterns();
        demonstrate_concepts();
        migration_demo::demonstrate_migration();
        performance_demo::benchmark_modules();
        demonstrate_module_benefits();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nKey Module Concepts:" << std::endl;
    std::cout << "• Module interfaces separate from implementations" << std::endl;
    std::cout << "• Explicit export control with 'export' keyword" << std::endl;
    std::cout << "• Module partitions for logical organization" << std::endl;
    std::cout << "• Improved compilation performance" << std::endl;
    std::cout << "• Better encapsulation than header files" << std::endl;
    std::cout << "• Seamless integration with existing code" << std::endl;
    
    return 0;
}

/*
REAL C++20 MODULE SYNTAX EXAMPLES:

// math_module.cppm (Module interface)
export module math;

export namespace math {
    double add(double a, double b);
    
    template<typename T>
    T multiply(T a, T b) {
        return a * b;
    }
    
    class Calculator {
    public:
        double calculate(double a, double b);
    };
}

// math_module.cpp (Module implementation)
module math;

double math::add(double a, double b) {
    return a + b;
}

double math::Calculator::calculate(double a, double b) {
    return add(a, b);
}

// main.cpp (Module consumer)
import math;
import std;

int main() {
    math::Calculator calc;
    auto result = calc.calculate(10, 5);
    std::cout << result << std::endl;
    return 0;
}

COMPILATION COMMANDS:
g++ -std=c++20 -fmodules-ts -c math_module.cppm
g++ -std=c++20 -fmodules-ts main.cpp math_module.o -o program
*/
