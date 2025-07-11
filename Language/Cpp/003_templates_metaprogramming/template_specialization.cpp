/*
 * TEMPLATE SPECIALIZATION - Advanced C++ Template Programming
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -g -O2 template_specialization.cpp -o template_specialization
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <typeinfo>
#include <map>
#include <list>

#define SPEC_INFO(msg) std::cout << "[SPECIALIZATION INFO] " << msg << std::endl
#define TYPE_INFO(T) std::cout << "[TYPE INFO] " << #T << " = " << typeid(T).name() << std::endl

namespace TemplateSpecialization {

// ============================================================================
// 1. FUNCTION TEMPLATE SPECIALIZATION
// ============================================================================

// Primary template
template<typename T>
void print_type_info(const T& value) {
    std::cout << "Generic type: " << typeid(T).name() << ", value: " << value << std::endl;
}

// Full specialization for int
template<>
void print_type_info<int>(const int& value) {
    std::cout << "Integer specialization: " << value << std::endl;
    std::cout << "  Size: " << sizeof(int) << " bytes" << std::endl;
    std::cout << "  Range: " << std::numeric_limits<int>::min() << " to " << std::numeric_limits<int>::max() << std::endl;
}

// Full specialization for std::string
template<>
void print_type_info<std::string>(const std::string& value) {
    std::cout << "String specialization: \"" << value << "\"" << std::endl;
    std::cout << "  Length: " << value.length() << std::endl;
    std::cout << "  Capacity: " << value.capacity() << std::endl;
}

// ============================================================================
// 2. CLASS TEMPLATE FULL SPECIALIZATION
// ============================================================================

// Primary template
template<typename T>
class DataProcessor {
private:
    T data_;

public:
    explicit DataProcessor(const T& data) : data_(data) {}
    
    void process() {
        std::cout << "Generic processing for: " << data_ << std::endl;
    }
    
    T get_result() const {
        return data_;
    }
};

// Full specialization for bool
template<>
class DataProcessor<bool> {
private:
    bool data_;

public:
    explicit DataProcessor(bool data) : data_(data) {}
    
    void process() {
        std::cout << "Boolean processing: " << (data_ ? "true" : "false") << std::endl;
        std::cout << "  Logical NOT: " << (!data_ ? "true" : "false") << std::endl;
    }
    
    bool get_result() const {
        return data_;
    }
    
    // Additional methods for bool specialization
    void toggle() {
        data_ = !data_;
        std::cout << "Toggled to: " << (data_ ? "true" : "false") << std::endl;
    }
};

// Full specialization for std::vector<int>
template<>
class DataProcessor<std::vector<int>> {
private:
    std::vector<int> data_;

public:
    explicit DataProcessor(const std::vector<int>& data) : data_(data) {}
    
    void process() {
        std::cout << "Vector<int> processing:" << std::endl;
        std::cout << "  Size: " << data_.size() << std::endl;
        std::cout << "  Elements: ";
        for (const auto& elem : data_) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
        
        if (!data_.empty()) {
            int sum = 0;
            for (int elem : data_) {
                sum += elem;
            }
            std::cout << "  Sum: " << sum << std::endl;
            std::cout << "  Average: " << static_cast<double>(sum) / data_.size() << std::endl;
        }
    }
    
    std::vector<int> get_result() const {
        return data_;
    }
    
    // Additional methods for vector specialization
    void sort() {
        std::sort(data_.begin(), data_.end());
        std::cout << "Vector sorted" << std::endl;
    }
    
    int find_max() const {
        if (data_.empty()) return 0;
        return *std::max_element(data_.begin(), data_.end());
    }
};

// ============================================================================
// 3. PARTIAL TEMPLATE SPECIALIZATION
// ============================================================================

// Primary template for containers
template<typename T, typename Container>
class ContainerAnalyzer {
public:
    void analyze(const Container& container) {
        std::cout << "Generic container analysis" << std::endl;
        std::cout << "Container type: " << typeid(Container).name() << std::endl;
        std::cout << "Element type: " << typeid(T).name() << std::endl;
    }
};

// Partial specialization for std::vector
template<typename T>
class ContainerAnalyzer<T, std::vector<T>> {
public:
    void analyze(const std::vector<T>& container) {
        std::cout << "Vector specialization analysis:" << std::endl;
        std::cout << "  Size: " << container.size() << std::endl;
        std::cout << "  Capacity: " << container.capacity() << std::endl;
        std::cout << "  Empty: " << (container.empty() ? "yes" : "no") << std::endl;
        
        if (!container.empty()) {
            std::cout << "  First element: " << container.front() << std::endl;
            std::cout << "  Last element: " << container.back() << std::endl;
        }
    }
};

// Partial specialization for std::list
template<typename T>
class ContainerAnalyzer<T, std::list<T>> {
public:
    void analyze(const std::list<T>& container) {
        std::cout << "List specialization analysis:" << std::endl;
        std::cout << "  Size: " << container.size() << std::endl;
        std::cout << "  Empty: " << (container.empty() ? "yes" : "no") << std::endl;
        
        if (!container.empty()) {
            std::cout << "  First element: " << container.front() << std::endl;
            std::cout << "  Last element: " << container.back() << std::endl;
        }
    }
};

// ============================================================================
// 4. POINTER SPECIALIZATIONS
// ============================================================================

// Primary template
template<typename T>
class SmartWrapper {
private:
    T value_;

public:
    explicit SmartWrapper(const T& value) : value_(value) {}
    
    void display() {
        std::cout << "Value wrapper: " << value_ << std::endl;
    }
    
    T get() const { return value_; }
};

// Specialization for pointer types
template<typename T>
class SmartWrapper<T*> {
private:
    T* ptr_;

public:
    explicit SmartWrapper(T* ptr) : ptr_(ptr) {}
    
    void display() {
        std::cout << "Pointer wrapper:" << std::endl;
        std::cout << "  Address: " << static_cast<void*>(ptr_) << std::endl;
        if (ptr_) {
            std::cout << "  Value: " << *ptr_ << std::endl;
        } else {
            std::cout << "  Value: nullptr" << std::endl;
        }
    }
    
    T* get() const { return ptr_; }
    
    // Additional methods for pointer specialization
    bool is_null() const {
        return ptr_ == nullptr;
    }
    
    T& dereference() {
        if (!ptr_) {
            throw std::runtime_error("Dereferencing null pointer");
        }
        return *ptr_;
    }
};

// ============================================================================
// 5. ARRAY SPECIALIZATIONS
// ============================================================================

// Primary template
template<typename T, size_t N>
class ArrayProcessor {
private:
    T data_[N];

public:
    ArrayProcessor() {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = T{};
        }
    }
    
    void fill(const T& value) {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = value;
        }
    }
    
    void print() {
        std::cout << "Array[" << N << "]: ";
        for (size_t i = 0; i < N; ++i) {
            std::cout << data_[i] << " ";
        }
        std::cout << std::endl;
    }
    
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    
    constexpr size_t size() const { return N; }
};

// Specialization for size 0 (empty array)
template<typename T>
class ArrayProcessor<T, 0> {
public:
    void fill(const T&) {
        std::cout << "Cannot fill empty array" << std::endl;
    }
    
    void print() {
        std::cout << "Empty array[]" << std::endl;
    }
    
    constexpr size_t size() const { return 0; }
};

// Specialization for size 1 (single element)
template<typename T>
class ArrayProcessor<T, 1> {
private:
    T data_;

public:
    ArrayProcessor() : data_(T{}) {}
    
    void fill(const T& value) {
        data_ = value;
    }
    
    void print() {
        std::cout << "Single element array: " << data_ << std::endl;
    }
    
    T& operator[](size_t) { return data_; }
    const T& operator[](size_t) const { return data_; }
    
    constexpr size_t size() const { return 1; }
    
    // Additional methods for single element
    T& get() { return data_; }
    const T& get() const { return data_; }
};

// ============================================================================
// 6. TEMPLATE SPECIALIZATION WITH SFINAE
// ============================================================================

// Primary template
template<typename T, typename Enable = void>
class TypeClassifier {
public:
    static std::string classify() {
        return "Unknown type";
    }
};

// Specialization for integral types
template<typename T>
class TypeClassifier<T, typename std::enable_if<std::is_integral<T>::value>::type> {
public:
    static std::string classify() {
        return "Integral type: " + std::string(typeid(T).name()) + 
               " (size: " + std::to_string(sizeof(T)) + " bytes)";
    }
};

// Specialization for floating point types
template<typename T>
class TypeClassifier<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
public:
    static std::string classify() {
        return "Floating point type: " + std::string(typeid(T).name()) + 
               " (size: " + std::to_string(sizeof(T)) + " bytes)";
    }
};

// Specialization for pointer types
template<typename T>
class TypeClassifier<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
public:
    static std::string classify() {
        return "Pointer type: " + std::string(typeid(T).name());
    }
};

// ============================================================================
// 7. ADVANCED SPECIALIZATION PATTERNS
// ============================================================================

// Template for function types
template<typename T>
struct FunctionTraits {
    static constexpr bool is_function = false;
    static constexpr size_t arity = 0;
};

// Specialization for function pointers
template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
    static constexpr bool is_function = true;
    static constexpr size_t arity = sizeof...(Args);
    
    static void print_info() {
        std::cout << "Function pointer analysis:" << std::endl;
        std::cout << "  Return type: " << typeid(R).name() << std::endl;
        std::cout << "  Argument count: " << arity << std::endl;
        std::cout << "  Is function: " << is_function << std::endl;
    }
};

// Specialization for member function pointers
template<typename R, typename C, typename... Args>
struct FunctionTraits<R(C::*)(Args...)> {
    using return_type = R;
    using class_type = C;
    using argument_types = std::tuple<Args...>;
    static constexpr bool is_function = true;
    static constexpr bool is_member_function = true;
    static constexpr size_t arity = sizeof...(Args);
    
    static void print_info() {
        std::cout << "Member function pointer analysis:" << std::endl;
        std::cout << "  Return type: " << typeid(R).name() << std::endl;
        std::cout << "  Class type: " << typeid(C).name() << std::endl;
        std::cout << "  Argument count: " << arity << std::endl;
        std::cout << "  Is member function: " << is_member_function << std::endl;
    }
};

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

void demonstrate_function_specialization() {
    std::cout << "\n=== FUNCTION TEMPLATE SPECIALIZATION ===" << std::endl;
    
    print_type_info(42);
    print_type_info(3.14);
    print_type_info(std::string("Hello World"));
    print_type_info('A');
}

void demonstrate_class_specialization() {
    std::cout << "\n=== CLASS TEMPLATE FULL SPECIALIZATION ===" << std::endl;
    
    DataProcessor<int> int_processor(42);
    int_processor.process();
    
    DataProcessor<bool> bool_processor(true);
    bool_processor.process();
    bool_processor.toggle();
    
    std::vector<int> vec = {1, 5, 3, 9, 2, 8};
    DataProcessor<std::vector<int>> vec_processor(vec);
    vec_processor.process();
    vec_processor.sort();
    std::cout << "Max element: " << vec_processor.find_max() << std::endl;
}

void demonstrate_partial_specialization() {
    std::cout << "\n=== PARTIAL TEMPLATE SPECIALIZATION ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    ContainerAnalyzer<int, std::vector<int>> vec_analyzer;
    vec_analyzer.analyze(vec);
    
    std::list<std::string> list = {"hello", "world", "template"};
    ContainerAnalyzer<std::string, std::list<std::string>> list_analyzer;
    list_analyzer.analyze(list);
}

void demonstrate_pointer_specialization() {
    std::cout << "\n=== POINTER SPECIALIZATION ===" << std::endl;
    
    SmartWrapper<int> value_wrapper(42);
    value_wrapper.display();
    
    int value = 100;
    SmartWrapper<int*> ptr_wrapper(&value);
    ptr_wrapper.display();
    std::cout << "Is null: " << ptr_wrapper.is_null() << std::endl;
    
    SmartWrapper<int*> null_wrapper(nullptr);
    null_wrapper.display();
    std::cout << "Is null: " << null_wrapper.is_null() << std::endl;
}

void demonstrate_array_specialization() {
    std::cout << "\n=== ARRAY SPECIALIZATION ===" << std::endl;
    
    ArrayProcessor<int, 5> normal_array;
    normal_array.fill(7);
    normal_array.print();
    
    ArrayProcessor<int, 0> empty_array;
    empty_array.print();
    
    ArrayProcessor<int, 1> single_array;
    single_array.fill(99);
    single_array.print();
    std::cout << "Single element: " << single_array.get() << std::endl;
}

void demonstrate_sfinae_specialization() {
    std::cout << "\n=== SFINAE SPECIALIZATION ===" << std::endl;
    
    std::cout << TypeClassifier<int>::classify() << std::endl;
    std::cout << TypeClassifier<double>::classify() << std::endl;
    std::cout << TypeClassifier<int*>::classify() << std::endl;
    std::cout << TypeClassifier<std::string>::classify() << std::endl;
}

void demonstrate_advanced_patterns() {
    std::cout << "\n=== ADVANCED SPECIALIZATION PATTERNS ===" << std::endl;
    
    // Function pointer analysis
    int (*func_ptr)(int, double) = nullptr;
    FunctionTraits<decltype(func_ptr)>::print_info();
    
    // Member function pointer analysis
    struct TestClass {
        void member_func(int, std::string) {}
    };
    
    void (TestClass::*member_ptr)(int, std::string) = &TestClass::member_func;
    FunctionTraits<decltype(member_ptr)>::print_info();
}

} // namespace TemplateSpecialization

int main() {
    std::cout << "=== TEMPLATE SPECIALIZATION TUTORIAL ===" << std::endl;
    
    try {
        TemplateSpecialization::demonstrate_function_specialization();
        TemplateSpecialization::demonstrate_class_specialization();
        TemplateSpecialization::demonstrate_partial_specialization();
        TemplateSpecialization::demonstrate_pointer_specialization();
        TemplateSpecialization::demonstrate_array_specialization();
        TemplateSpecialization::demonstrate_sfinae_specialization();
        TemplateSpecialization::demonstrate_advanced_patterns();
        
        std::cout << "\n=== TUTORIAL COMPLETED ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
