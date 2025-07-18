/*
 * CLASS TEMPLATES - Advanced C++ Template Programming
 * 
 * This comprehensive tutorial covers:
 * 1. Basic Class Templates
 * 2. Template Parameters (type, non-type, template)
 * 3. Class Template Specialization
 * 4. Partial Template Specialization
 * 5. Member Templates
 * 6. Template Aliases
 * 7. Template Friends
 * 8. Template Inheritance
 * 9. Policy-Based Design
 * 10. Modern C++ Class Template Features
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -g -O2 class_templates.cpp -o class_templates
 * Advanced: g++ -std=c++20 -Wall -Wextra -g -O2 class_templates.cpp -o class_templates
 */

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <chrono>
#include <algorithm>
#include <functional>
#include <typeinfo>
#include <cassert>
#include <array>

// Debug macros
#define DEBUG_CLASS(x) std::cout << "[CLASS DEBUG] " << #x << " = " << (x) << std::endl
#define CLASS_INFO(msg) std::cout << "[CLASS INFO] " << msg << std::endl
#define TYPE_INFO(T) std::cout << "[TYPE INFO] " << #T << " = " << typeid(T).name() << std::endl

// Performance measurement
#define MEASURE_TIME(code) \
    do { \
        auto start = std::chrono::high_resolution_clock::now(); \
        code; \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); \
        std::cout << "[PERFORMANCE] " << #code << " took " << duration.count() << " microseconds" << std::endl; \
    } while(0)

namespace ClassTemplates {

// ============================================================================
// 1. BASIC CLASS TEMPLATES
// ============================================================================

// Simple class template
template<typename T>
class Container {
private:
    T* data;
    size_t size_;
    size_t capacity_;

public:
    // Constructor
    explicit Container(size_t initial_capacity = 10) 
        : size_(0), capacity_(initial_capacity) {
        data = new T[capacity_];
        CLASS_INFO("Container<" + std::string(typeid(T).name()) + "> constructed");
    }
    
    // Copy constructor
    Container(const Container& other) 
        : size_(other.size_), capacity_(other.capacity_) {
        data = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            data[i] = other.data[i];
        }
        CLASS_INFO("Container copy constructed");
    }
    
    // Move constructor
    Container(Container&& other) noexcept 
        : data(other.data), size_(other.size_), capacity_(other.capacity_) {
        other.data = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        CLASS_INFO("Container move constructed");
    }
    
    // Destructor
    ~Container() {
        delete[] data;
        CLASS_INFO("Container destructed");
    }
    
    // Assignment operators
    Container& operator=(const Container& other) {
        if (this != &other) {
            delete[] data;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }
    
    Container& operator=(Container&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    // Member functions
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            resize();
        }
        data[size_++] = value;
    }
    
    void push_back(T&& value) {
        if (size_ >= capacity_) {
            resize();
        }
        data[size_++] = std::move(value);
    }
    
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    
    // Iterator support
    T* begin() { return data; }
    T* end() { return data + size_; }
    const T* begin() const { return data; }
    const T* end() const { return data + size_; }

private:
    void resize() {
        capacity_ *= 2;
        T* new_data = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data[i]);
        }
        delete[] data;
        data = new_data;
        CLASS_INFO("Container resized to capacity: " + std::to_string(capacity_));
    }
};

// ============================================================================
// 2. TEMPLATE PARAMETERS (TYPE, NON-TYPE, TEMPLATE)
// ============================================================================

// Class template with multiple template parameters
template<typename T, size_t N, typename Allocator = std::allocator<T>>
class FixedArray {
private:
    T data[N];
    Allocator alloc;

public:
    using value_type = T;
    using size_type = size_t;
    static constexpr size_type array_size = N;
    
    FixedArray() {
        CLASS_INFO("FixedArray<" + std::string(typeid(T).name()) + ", " + std::to_string(N) + "> constructed");
    }
    
    T& operator[](size_t index) {
        assert(index < N);
        return data[index];
    }
    
    const T& operator[](size_t index) const {
        assert(index < N);
        return data[index];
    }
    
    constexpr size_t size() const { return N; }
    
    T* begin() { return data; }
    T* end() { return data + N; }
    const T* begin() const { return data; }
    const T* end() const { return data + N; }
};

// Template template parameter
template<template<typename> class Container, typename T>
class ContainerAdapter {
private:
    Container<T> container;

public:
    void add(const T& value) {
        container.push_back(value);
    }
    
    size_t size() const {
        return container.size();
    }
    
    void print() const {
        std::cout << "Container contents: ";
        for (const auto& item : container) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// ============================================================================
// 3. CLASS TEMPLATE SPECIALIZATION
// ============================================================================

// Primary template
template<typename T>
class TypeProcessor {
public:
    void process(const T& value) {
        std::cout << "Processing generic type: " << value << std::endl;
    }
    
    std::string get_type_info() const {
        return "Generic type: " + std::string(typeid(T).name());
    }
};

// Full specialization for int
template<>
class TypeProcessor<int> {
public:
    void process(const int& value) {
        std::cout << "Processing integer: " << value << " (specialized)" << std::endl;
        std::cout << "Integer operations available" << std::endl;
    }
    
    std::string get_type_info() const {
        return "Specialized for integer type";
    }
    
    // Additional methods for int specialization
    void increment(int& value) {
        ++value;
        std::cout << "Incremented to: " << value << std::endl;
    }
};

// Full specialization for std::string
template<>
class TypeProcessor<std::string> {
public:
    void process(const std::string& value) {
        std::cout << "Processing string: \"" << value << "\" (specialized)" << std::endl;
        std::cout << "String length: " << value.length() << std::endl;
    }
    
    std::string get_type_info() const {
        return "Specialized for string type";
    }
    
    // Additional methods for string specialization
    void to_upper(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        std::cout << "Converted to uppercase: " << str << std::endl;
    }
};

// ============================================================================
// 4. PARTIAL TEMPLATE SPECIALIZATION
// ============================================================================

// Primary template for pairs
template<typename T, typename U>
class Pair {
private:
    T first_;
    U second_;

public:
    Pair(const T& first, const U& second) : first_(first), second_(second) {}
    
    T& first() { return first_; }
    U& second() { return second_; }
    const T& first() const { return first_; }
    const U& second() const { return second_; }
    
    void print() const {
        std::cout << "Pair: (" << first_ << ", " << second_ << ")" << std::endl;
    }
};

// Partial specialization for pairs with same type
template<typename T>
class Pair<T, T> {
private:
    T first_;
    T second_;

public:
    Pair(const T& first, const T& second) : first_(first), second_(second) {}
    
    T& first() { return first_; }
    T& second() { return second_; }
    const T& first() const { return first_; }
    const T& second() const { return second_; }
    
    void print() const {
        std::cout << "Same-type Pair: (" << first_ << ", " << second_ << ")" << std::endl;
    }
    
    // Additional functionality for same-type pairs
    void swap() {
        std::swap(first_, second_);
        std::cout << "Swapped pair values" << std::endl;
    }
    
    bool are_equal() const {
        return first_ == second_;
    }
};

// Partial specialization for pointer types
template<typename T>
class Pair<T*, T*> {
private:
    T* first_;
    T* second_;

public:
    Pair(T* first, T* second) : first_(first), second_(second) {}
    
    T*& first() { return first_; }
    T*& second() { return second_; }
    T* const& first() const { return first_; }
    T* const& second() const { return second_; }
    
    void print() const {
        std::cout << "Pointer Pair: (" << static_cast<void*>(first_) 
                  << ", " << static_cast<void*>(second_) << ")" << std::endl;
        if (first_ && second_) {
            std::cout << "Values: (" << *first_ << ", " << *second_ << ")" << std::endl;
        }
    }
    
    // Additional functionality for pointer pairs
    bool both_valid() const {
        return first_ != nullptr && second_ != nullptr;
    }
};

// ============================================================================
// 5. MEMBER TEMPLATES
// ============================================================================

template<typename T>
class SmartContainer {
private:
    std::vector<T> data;

public:
    // Member template constructor
    template<typename Iterator>
    SmartContainer(Iterator first, Iterator last) : data(first, last) {
        CLASS_INFO("SmartContainer constructed from iterators");
    }
    
    // Member template function
    template<typename U>
    void add_converted(const U& value) {
        data.push_back(static_cast<T>(value));
        CLASS_INFO("Added converted value");
    }
    
    // Member template with SFINAE
    template<typename U>
    typename std::enable_if<std::is_convertible<U, T>::value, void>::type
    safe_add(const U& value) {
        data.push_back(static_cast<T>(value));
        CLASS_INFO("Safely added convertible value");
    }
    
    // Member template operator
    template<typename U>
    SmartContainer& operator+=(const SmartContainer<U>& other) {
        for (const auto& item : other.data) {
            data.push_back(static_cast<T>(item));
        }
        return *this;
    }
    
    // Member template with perfect forwarding
    template<typename... Args>
    void emplace_back(Args&&... args) {
        data.emplace_back(std::forward<Args>(args)...);
        CLASS_INFO("Emplaced value with perfect forwarding");
    }
    
    size_t size() const { return data.size(); }
    
    void print() const {
        std::cout << "SmartContainer contents: ";
        for (const auto& item : data) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
    
    // Friend access for template operator
    template<typename U>
    friend class SmartContainer;
};

// ============================================================================
// 6. TEMPLATE ALIASES
// ============================================================================

// Basic template alias
template<typename T>
using Vector = std::vector<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

// Complex template alias
template<typename Key, typename Value>
using StringMap = std::map<std::string, std::pair<Key, Value>>;

// Template alias with default parameters
template<typename T, size_t N = 10>
using FixedVector = FixedArray<T, N>;

// Template alias for function pointers
template<typename R, typename... Args>
using FunctionPtr = R(*)(Args...);

// ============================================================================
// 7. TEMPLATE FRIENDS
// ============================================================================

template<typename T>
class Matrix;

template<typename T>
Matrix<T> operator+(const Matrix<T>& lhs, const Matrix<T>& rhs);

template<typename T>
class Matrix {
private:
    std::vector<std::vector<T>> data;
    size_t rows_, cols_;

public:
    Matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
        data.resize(rows, std::vector<T>(cols, T{}));
    }
    
    T& operator()(size_t row, size_t col) {
        return data[row][col];
    }
    
    const T& operator()(size_t row, size_t col) const {
        return data[row][col];
    }
    
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    
    // Template friend function
    friend Matrix<T> operator+<T>(const Matrix<T>& lhs, const Matrix<T>& rhs);
    
    // Template friend class
    template<typename U>
    friend class MatrixProcessor;
    
    void print() const {
        std::cout << "Matrix " << rows_ << "x" << cols_ << ":" << std::endl;
        for (size_t i = 0; i < rows_; ++i) {
            for (size_t j = 0; j < cols_; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

template<typename T>
Matrix<T> operator+(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    if (lhs.rows_ != rhs.rows_ || lhs.cols_ != rhs.cols_) {
        throw std::invalid_argument("Matrix dimensions must match");
    }
    
    Matrix<T> result(lhs.rows_, lhs.cols_);
    for (size_t i = 0; i < lhs.rows_; ++i) {
        for (size_t j = 0; j < lhs.cols_; ++j) {
            result.data[i][j] = lhs.data[i][j] + rhs.data[i][j];
        }
    }
    return result;
}

template<typename T>
class MatrixProcessor {
public:
    static T sum_all_elements(const Matrix<T>& matrix) {
        T sum = T{};
        for (const auto& row : matrix.data) {
            for (const auto& element : row) {
                sum += element;
            }
        }
        return sum;
    }
};

// ============================================================================
// 8. TEMPLATE INHERITANCE
// ============================================================================

// Base template class
template<typename T>
class Shape {
protected:
    T x_, y_;

public:
    Shape(T x, T y) : x_(x), y_(y) {}
    virtual ~Shape() = default;
    
    virtual T area() const = 0;
    virtual void print() const {
        std::cout << "Shape at (" << x_ << ", " << y_ << ")" << std::endl;
    }
    
    T get_x() const { return x_; }
    T get_y() const { return y_; }
};

// Derived template class
template<typename T>
class Rectangle : public Shape<T> {
private:
    T width_, height_;

public:
    Rectangle(T x, T y, T width, T height) 
        : Shape<T>(x, y), width_(width), height_(height) {}
    
    T area() const override {
        return width_ * height_;
    }
    
    void print() const override {
        std::cout << "Rectangle at (" << this->x_ << ", " << this->y_ 
                  << ") with width " << width_ << " and height " << height_ << std::endl;
    }
};

template<typename T>
class Circle : public Shape<T> {
private:
    T radius_;

public:
    Circle(T x, T y, T radius) : Shape<T>(x, y), radius_(radius) {}
    
    T area() const override {
        return static_cast<T>(3.14159) * radius_ * radius_;
    }
    
    void print() const override {
        std::cout << "Circle at (" << this->x_ << ", " << this->y_ 
                  << ") with radius " << radius_ << std::endl;
    }
};

// ============================================================================
// 9. POLICY-BASED DESIGN
// ============================================================================

// Storage policies
template<typename T>
class VectorStorage {
private:
    std::vector<T> data_;

public:
    void add(const T& item) { data_.push_back(item); }
    size_t size() const { return data_.size(); }
    T& get(size_t index) { return data_[index]; }
    const T& get(size_t index) const { return data_[index]; }
    void clear() { data_.clear(); }
};

template<typename T, size_t N>
class ArrayStorage {
private:
    T data_[N];
    size_t size_;

public:
    ArrayStorage() : size_(0) {}
    
    void add(const T& item) {
        if (size_ < N) {
            data_[size_++] = item;
        }
    }
    
    size_t size() const { return size_; }
    T& get(size_t index) { return data_[index]; }
    const T& get(size_t index) const { return data_[index]; }
    void clear() { size_ = 0; }
};

// Threading policies
class SingleThreaded {
public:
    void lock() {}
    void unlock() {}
};

class MultiThreaded {
private:
    mutable std::mutex mutex_;

public:
    void lock() const { mutex_.lock(); }
    void unlock() const { mutex_.unlock(); }
};

// Policy-based container
template<typename T, 
         template<typename> class StoragePolicy = VectorStorage,
         class ThreadingPolicy = SingleThreaded>
class PolicyContainer : private StoragePolicy<T>, private ThreadingPolicy {
public:
    void add(const T& item) {
        this->lock();
        StoragePolicy<T>::add(item);
        this->unlock();
    }
    
    size_t size() const {
        this->lock();
        size_t result = StoragePolicy<T>::size();
        this->unlock();
        return result;
    }
    
    T get(size_t index) const {
        this->lock();
        T result = StoragePolicy<T>::get(index);
        this->unlock();
        return result;
    }
    
    void print() const {
        this->lock();
        std::cout << "PolicyContainer contents (" << this->size() << " items): ";
        for (size_t i = 0; i < StoragePolicy<T>::size(); ++i) {
            std::cout << StoragePolicy<T>::get(i) << " ";
        }
        std::cout << std::endl;
        this->unlock();
    }
};

// ============================================================================
// 10. MODERN C++ CLASS TEMPLATE FEATURES
// ============================================================================

// Class template argument deduction (CTAD) - C++17
template<typename T>
class ModernContainer {
private:
    std::vector<T> data_;

public:
    // Deduction guide will be automatically generated
    ModernContainer(std::initializer_list<T> init) : data_(init) {}
    
    template<typename Iterator>
    ModernContainer(Iterator first, Iterator last) : data_(first, last) {}
    
    void print() const {
        std::cout << "ModernContainer: ";
        for (const auto& item : data_) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// Explicit deduction guide
template<typename Iterator>
ModernContainer(Iterator, Iterator) -> ModernContainer<typename std::iterator_traits<Iterator>::value_type>;

// Variable templates in class context
template<typename T>
class TypeTraits {
public:
    static constexpr bool is_integral = std::is_integral_v<T>;
    static constexpr bool is_floating_point = std::is_floating_point_v<T>;
    static constexpr size_t size = sizeof(T);
    
    static void print_info() {
        std::cout << "Type " << typeid(T).name() << ":" << std::endl;
        std::cout << "  Is integral: " << is_integral << std::endl;
        std::cout << "  Is floating point: " << is_floating_point << std::endl;
        std::cout << "  Size: " << size << " bytes" << std::endl;
    }
};

// ============================================================================
// DEMONSTRATION FUNCTIONS
// ============================================================================

void demonstrate_basic_templates() {
    std::cout << "\n=== BASIC CLASS TEMPLATES ===" << std::endl;
    
    Container<int> int_container;
    int_container.push_back(1);
    int_container.push_back(2);
    int_container.push_back(3);
    
    std::cout << "Container size: " << int_container.size() << std::endl;
    for (size_t i = 0; i < int_container.size(); ++i) {
        std::cout << "Element " << i << ": " << int_container.at(i) << std::endl;
    }
    
    Container<std::string> string_container;
    string_container.push_back("Hello");
    string_container.push_back("World");
    
    std::cout << "String container: ";
    for (const auto& str : string_container) {
        std::cout << str << " ";
    }
    std::cout << std::endl;
}

void demonstrate_template_parameters() {
    std::cout << "\n=== TEMPLATE PARAMETERS ===" << std::endl;
    
    FixedArray<int, 5> fixed_array;
    for (size_t i = 0; i < fixed_array.size(); ++i) {
        fixed_array[i] = static_cast<int>(i * i);
    }
    
    std::cout << "Fixed array: ";
    for (const auto& item : fixed_array) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    
    ContainerAdapter<std::vector, int> adapter;
    adapter.add(10);
    adapter.add(20);
    adapter.add(30);
    adapter.print();
}

void demonstrate_specialization() {
    std::cout << "\n=== CLASS TEMPLATE SPECIALIZATION ===" << std::endl;
    
    TypeProcessor<int> int_processor;
    int_processor.process(42);
    std::cout << int_processor.get_type_info() << std::endl;
    int value = 10;
    int_processor.increment(value);
    
    TypeProcessor<std::string> string_processor;
    string_processor.process("hello world");
    std::cout << string_processor.get_type_info() << std::endl;
    std::string str = "hello";
    string_processor.to_upper(str);
    
    TypeProcessor<double> double_processor;
    double_processor.process(3.14159);
    std::cout << double_processor.get_type_info() << std::endl;
}

void demonstrate_partial_specialization() {
    std::cout << "\n=== PARTIAL TEMPLATE SPECIALIZATION ===" << std::endl;
    
    Pair<int, std::string> mixed_pair(42, "hello");
    mixed_pair.print();
    
    Pair<int, int> same_pair(10, 20);
    same_pair.print();
    std::cout << "Are equal: " << same_pair.are_equal() << std::endl;
    same_pair.swap();
    same_pair.print();
    
    int a = 100, b = 200;
    Pair<int*, int*> pointer_pair(&a, &b);
    pointer_pair.print();
    std::cout << "Both valid: " << pointer_pair.both_valid() << std::endl;
}

void demonstrate_member_templates() {
    std::cout << "\n=== MEMBER TEMPLATES ===" << std::endl;
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    SmartContainer<int> container(vec.begin(), vec.end());
    container.print();
    
    container.add_converted(3.14);
    container.safe_add(42.5);
    container.emplace_back(100);
    container.print();
}

void demonstrate_template_aliases() {
    std::cout << "\n=== TEMPLATE ALIASES ===" << std::endl;
    
    Vector<int> int_vector = {1, 2, 3, 4, 5};
    std::cout << "Vector size: " << int_vector.size() << std::endl;
    
    SharedPtr<std::string> str_ptr = std::make_shared<std::string>("Hello");
    std::cout << "Shared string: " << *str_ptr << std::endl;
    
    FixedVector<double, 3> fixed_vec;
    fixed_vec[0] = 1.1;
    fixed_vec[1] = 2.2;
    fixed_vec[2] = 3.3;
    
    std::cout << "Fixed vector: ";
    for (const auto& item : fixed_vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

void demonstrate_template_friends() {
    std::cout << "\n=== TEMPLATE FRIENDS ===" << std::endl;
    
    Matrix<int> matrix1(2, 2);
    matrix1(0, 0) = 1; matrix1(0, 1) = 2;
    matrix1(1, 0) = 3; matrix1(1, 1) = 4;
    
    Matrix<int> matrix2(2, 2);
    matrix2(0, 0) = 5; matrix2(0, 1) = 6;
    matrix2(1, 0) = 7; matrix2(1, 1) = 8;
    
    std::cout << "Matrix 1:" << std::endl;
    matrix1.print();
    
    std::cout << "Matrix 2:" << std::endl;
    matrix2.print();
    
    Matrix<int> result = matrix1 + matrix2;
    std::cout << "Sum:" << std::endl;
    result.print();
    
    int sum = MatrixProcessor<int>::sum_all_elements(result);
    std::cout << "Sum of all elements: " << sum << std::endl;
}

void demonstrate_template_inheritance() {
    std::cout << "\n=== TEMPLATE INHERITANCE ===" << std::endl;
    
    Rectangle<double> rect(0.0, 0.0, 5.0, 3.0);
    rect.print();
    std::cout << "Area: " << rect.area() << std::endl;
    
    Circle<double> circle(1.0, 1.0, 2.5);
    circle.print();
    std::cout << "Area: " << circle.area() << std::endl;
    
    std::vector<std::unique_ptr<Shape<double>>> shapes;
    shapes.push_back(std::make_unique<Rectangle<double>>(0, 0, 4, 6));
    shapes.push_back(std::make_unique<Circle<double>>(2, 2, 3));
    
    double total_area = 0.0;
    for (const auto& shape : shapes) {
        shape->print();
        total_area += shape->area();
    }
    std::cout << "Total area: " << total_area << std::endl;
}

void demonstrate_policy_based_design() {
    std::cout << "\n=== POLICY-BASED DESIGN ===" << std::endl;
    
    PolicyContainer<int, VectorStorage, SingleThreaded> vector_container;
    vector_container.add(1);
    vector_container.add(2);
    vector_container.add(3);
    vector_container.print();
    
    PolicyContainer<int, ArrayStorage, SingleThreaded> array_container;
    array_container.add(10);
    array_container.add(20);
    array_container.add(30);
    array_container.print();
}

void demonstrate_modern_features() {
    std::cout << "\n=== MODERN C++ FEATURES ===" << std::endl;
    
    // CTAD (Class Template Argument Deduction)
    ModernContainer container1{1, 2, 3, 4, 5};  // Deduces ModernContainer<int>
    container1.print();
    
    std::vector<double> vec = {1.1, 2.2, 3.3};
    ModernContainer container2(vec.begin(), vec.end());  // Deduces ModernContainer<double>
    container2.print();
    
    // Type traits
    TypeTraits<int>::print_info();
    TypeTraits<double>::print_info();
    TypeTraits<std::string>::print_info();
}

void performance_comparison() {
    std::cout << "\n=== PERFORMANCE COMPARISON ===" << std::endl;
    
    const int iterations = 100000;
    
    // Template container
    MEASURE_TIME({
        Container<int> container;
        for (int i = 0; i < iterations; ++i) {
            container.push_back(i);
        }
    });
    
    // Standard vector
    MEASURE_TIME({
        std::vector<int> vector;
        for (int i = 0; i < iterations; ++i) {
            vector.push_back(i);
        }
    });
    
    // Fixed array (when size is known)
    MEASURE_TIME({
        FixedArray<int, 100000> fixed_array;
        for (int i = 0; i < 100000; ++i) {
            fixed_array[i] = i;
        }
    });
}

} // namespace ClassTemplates

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=== CLASS TEMPLATES COMPREHENSIVE TUTORIAL ===" << std::endl;
    
    try {
        ClassTemplates::demonstrate_basic_templates();
        ClassTemplates::demonstrate_template_parameters();
        ClassTemplates::demonstrate_specialization();
        ClassTemplates::demonstrate_partial_specialization();
        ClassTemplates::demonstrate_member_templates();
        ClassTemplates::demonstrate_template_aliases();
        ClassTemplates::demonstrate_template_friends();
        ClassTemplates::demonstrate_template_inheritance();
        ClassTemplates::demonstrate_policy_based_design();
        ClassTemplates::demonstrate_modern_features();
        ClassTemplates::performance_comparison();
        
        std::cout << "\n=== TUTORIAL COMPLETED SUCCESSFULLY ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

/*
 * COMPILATION NOTES:
 * 
 * Basic compilation:
 * g++ -std=c++17 -Wall -Wextra -g -O2 class_templates.cpp -o class_templates
 * 
 * With all warnings:
 * g++ -std=c++17 -Wall -Wextra -Wpedantic -g -O2 class_templates.cpp -o class_templates
 * 
 * For C++20 features:
 * g++ -std=c++20 -Wall -Wextra -g -O2 class_templates.cpp -o class_templates
 * 
 * Debug version:
 * g++ -std=c++17 -Wall -Wextra -g -O0 -DDEBUG class_templates.cpp -o class_templates_debug
 * 
 * LEARNING OUTCOMES:
 * 
 * After studying this code, you should understand:
 * 1. How to design and implement class templates
 * 2. Different types of template parameters
 * 3. Full and partial template specialization
 * 4. Member templates and their use cases
 * 5. Template aliases and their benefits
 * 6. Template friends and access control
 * 7. Template inheritance patterns
 * 8. Policy-based design with templates
 * 9. Modern C++ class template features
 * 10. Performance considerations with templates
 * 
 * ADVANCED TOPICS COVERED:
 * - Template parameter types (type, non-type, template)
 * - Partial specialization techniques
 * - Member template functions and constructors
 * - Template friend functions and classes
 * - Policy-based design patterns
 * - CTAD (Class Template Argument Deduction)
 * - Template metaprogramming with classes
 * - Modern C++ template features
 * 
 * This tutorial provides comprehensive coverage of class template
 * programming in modern C++, from basic concepts to advanced techniques.
 */
