/*
 * DESIGN PATTERNS - STRATEGY AND POLICY-BASED DESIGN
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra strategy_policy_based.cpp -o strategy
 */

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. CLASSIC STRATEGY PATTERN
// =============================================================================

class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
    virtual std::string name() const = 0;
};

class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j < data.size() - i - 1; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
    
    std::string name() const override { return "Bubble Sort"; }
};

class QuickSort : public SortStrategy {
private:
    void quicksort(std::vector<int>& data, int low, int high) {
        if (low < high) {
            int pi = partition(data, low, high);
            quicksort(data, low, pi - 1);
            quicksort(data, pi + 1, high);
        }
    }
    
    int partition(std::vector<int>& data, int low, int high) {
        int pivot = data[high];
        int i = low - 1;
        
        for (int j = low; j < high; ++j) {
            if (data[j] < pivot) {
                ++i;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[high]);
        return i + 1;
    }
    
public:
    void sort(std::vector<int>& data) override {
        if (!data.empty()) {
            quicksort(data, 0, static_cast<int>(data.size()) - 1);
        }
    }
    
    std::string name() const override { return "Quick Sort"; }
};

class SortContext {
private:
    std::unique_ptr<SortStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<SortStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void sort(std::vector<int>& data) {
        if (strategy_) {
            std::cout << "Using " << strategy_->name() << std::endl;
            strategy_->sort(data);
        }
    }
};

void demonstrate_classic_strategy() {
    SECTION("Classic Strategy Pattern");
    
    std::vector<int> data1 = {64, 34, 25, 12, 22, 11, 90};
    std::vector<int> data2 = data1;
    
    SortContext context;
    
    // Use bubble sort
    context.setStrategy(std::make_unique<BubbleSort>());
    context.sort(data1);
    std::cout << "Result: ";
    for (int x : data1) std::cout << x << " ";
    std::cout << std::endl;
    
    // Use quick sort
    context.setStrategy(std::make_unique<QuickSort>());
    context.sort(data2);
    std::cout << "Result: ";
    for (int x : data2) std::cout << x << " ";
    std::cout << std::endl;
}

// =============================================================================
// 2. FUNCTION-BASED STRATEGY
// =============================================================================

class FunctionBasedSorter {
private:
    std::function<void(std::vector<int>&)> strategy_;
    std::string strategy_name_;
    
public:
    void setStrategy(std::function<void(std::vector<int>&)> strategy, const std::string& name) {
        strategy_ = strategy;
        strategy_name_ = name;
    }
    
    void sort(std::vector<int>& data) {
        if (strategy_) {
            std::cout << "Using " << strategy_name_ << std::endl;
            strategy_(data);
        }
    }
};

void demonstrate_function_strategy() {
    SECTION("Function-based Strategy");
    
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    FunctionBasedSorter sorter;
    
    // Use STL sort
    sorter.setStrategy([](std::vector<int>& data) {
        std::sort(data.begin(), data.end());
    }, "STL Sort");
    
    sorter.sort(data);
    std::cout << "Result: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Use reverse sort
    data = {64, 34, 25, 12, 22, 11, 90};
    sorter.setStrategy([](std::vector<int>& data) {
        std::sort(data.begin(), data.end(), std::greater<int>());
    }, "Reverse Sort");
    
    sorter.sort(data);
    std::cout << "Result: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
}

// =============================================================================
// 3. POLICY-BASED DESIGN
// =============================================================================

// Storage policies
template<typename T>
class VectorStorage {
private:
    std::vector<T> data_;
    
public:
    void add(const T& item) { data_.push_back(item); }
    void remove(size_t index) { if (index < data_.size()) data_.erase(data_.begin() + index); }
    T& get(size_t index) { return data_[index]; }
    size_t size() const { return data_.size(); }
    void clear() { data_.clear(); }
};

template<typename T>
class ListStorage {
private:
    std::list<T> data_;
    
public:
    void add(const T& item) { data_.push_back(item); }
    void remove(size_t index) {
        if (index < data_.size()) {
            auto it = data_.begin();
            std::advance(it, index);
            data_.erase(it);
        }
    }
    T& get(size_t index) {
        auto it = data_.begin();
        std::advance(it, index);
        return *it;
    }
    size_t size() const { return data_.size(); }
    void clear() { data_.clear(); }
};

// Threading policies
class SingleThreaded {
public:
    void lock() {}
    void unlock() {}
};

class MultiThreaded {
private:
    std::mutex mutex_;
    
public:
    void lock() { mutex_.lock(); }
    void unlock() { mutex_.unlock(); }
};

// Container with policies
template<typename T, 
         template<typename> class StoragePolicy = VectorStorage,
         class ThreadingPolicy = SingleThreaded>
class Container : private StoragePolicy<T>, private ThreadingPolicy {
public:
    void add(const T& item) {
        this->lock();
        StoragePolicy<T>::add(item);
        this->unlock();
    }
    
    void remove(size_t index) {
        this->lock();
        StoragePolicy<T>::remove(index);
        this->unlock();
    }
    
    T get(size_t index) {
        this->lock();
        T result = StoragePolicy<T>::get(index);
        this->unlock();
        return result;
    }
    
    size_t size() const {
        return StoragePolicy<T>::size();
    }
};

void demonstrate_policy_based_design() {
    SECTION("Policy-based Design");
    
    // Vector-based, single-threaded container
    Container<int, VectorStorage, SingleThreaded> vector_container;
    vector_container.add(1);
    vector_container.add(2);
    vector_container.add(3);
    
    std::cout << "Vector container size: " << vector_container.size() << std::endl;
    std::cout << "First element: " << vector_container.get(0) << std::endl;
    
    // List-based, multi-threaded container
    Container<std::string, ListStorage, MultiThreaded> list_container;
    list_container.add("hello");
    list_container.add("world");
    
    std::cout << "List container size: " << list_container.size() << std::endl;
    std::cout << "First element: " << list_container.get(0) << std::endl;
}

// =============================================================================
// 4. TEMPLATE STRATEGY PATTERN
// =============================================================================

template<typename SortStrategy>
class TemplateSorter {
private:
    SortStrategy strategy_;
    
public:
    void sort(std::vector<int>& data) {
        strategy_(data);
    }
};

struct BubbleSortPolicy {
    void operator()(std::vector<int>& data) {
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j < data.size() - i - 1; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
};

struct STLSortPolicy {
    void operator()(std::vector<int>& data) {
        std::sort(data.begin(), data.end());
    }
};

void demonstrate_template_strategy() {
    SECTION("Template Strategy Pattern");
    
    std::vector<int> data1 = {64, 34, 25, 12, 22, 11, 90};
    std::vector<int> data2 = data1;
    
    TemplateSorter<BubbleSortPolicy> bubble_sorter;
    bubble_sorter.sort(data1);
    std::cout << "Bubble sort result: ";
    for (int x : data1) std::cout << x << " ";
    std::cout << std::endl;
    
    TemplateSorter<STLSortPolicy> stl_sorter;
    stl_sorter.sort(data2);
    std::cout << "STL sort result: ";
    for (int x : data2) std::cout << x << " ";
    std::cout << std::endl;
}

// =============================================================================
// 5. ADVANCED POLICY COMPOSITION
// =============================================================================

// Logging policies
class NoLogging {
public:
    template<typename T>
    void log(const T&) {}
};

class ConsoleLogging {
public:
    template<typename T>
    void log(const T& message) {
        std::cout << "[LOG] " << message << std::endl;
    }
};

// Validation policies
class NoValidation {
public:
    template<typename T>
    bool validate(const T&) { return true; }
};

class RangeValidation {
private:
    int min_, max_;
    
public:
    RangeValidation(int min = 0, int max = 100) : min_(min), max_(max) {}
    
    bool validate(int value) {
        return value >= min_ && value <= max_;
    }
};

template<typename T,
         class LoggingPolicy = NoLogging,
         class ValidationPolicy = NoValidation>
class SmartContainer : private LoggingPolicy, private ValidationPolicy {
private:
    std::vector<T> data_;
    
public:
    bool add(const T& item) {
        this->log("Attempting to add item");
        
        if (!this->validate(item)) {
            this->log("Validation failed");
            return false;
        }
        
        data_.push_back(item);
        this->log("Item added successfully");
        return true;
    }
    
    size_t size() const { return data_.size(); }
    
    T get(size_t index) const {
        if (index < data_.size()) {
            return data_[index];
        }
        throw std::out_of_range("Index out of range");
    }
};

void demonstrate_policy_composition() {
    SECTION("Advanced Policy Composition");
    
    // Container with logging and validation
    SmartContainer<int, ConsoleLogging, RangeValidation> container;
    
    std::cout << "Adding valid values:" << std::endl;
    container.add(50);
    container.add(75);
    
    std::cout << "\nAdding invalid values:" << std::endl;
    container.add(150);  // Should fail validation
    container.add(-10);  // Should fail validation
    
    std::cout << "\nContainer size: " << container.size() << std::endl;
    std::cout << "First element: " << container.get(0) << std::endl;
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison");
    
    const size_t data_size = 10000;
    const int iterations = 100;
    
    // Generate test data
    std::vector<int> test_data(data_size);
    std::iota(test_data.begin(), test_data.end(), 1);
    std::random_shuffle(test_data.begin(), test_data.end());
    
    // Virtual function strategy
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto data = test_data;
        SortContext context;
        context.setStrategy(std::make_unique<QuickSort>());
        context.sort(data);
    }
    auto virtual_time = std::chrono::high_resolution_clock::now() - start;
    
    // Template strategy
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto data = test_data;
        TemplateSorter<STLSortPolicy> sorter;
        sorter.sort(data);
    }
    auto template_time = std::chrono::high_resolution_clock::now() - start;
    
    // Function strategy
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto data = test_data;
        std::sort(data.begin(), data.end());
    }
    auto function_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance Results (" << iterations << " iterations):" << std::endl;
    std::cout << "Virtual function: " << std::chrono::duration_cast<std::chrono::milliseconds>(virtual_time).count() << " ms" << std::endl;
    std::cout << "Template strategy: " << std::chrono::duration_cast<std::chrono::milliseconds>(template_time).count() << " ms" << std::endl;
    std::cout << "Direct function: " << std::chrono::duration_cast<std::chrono::milliseconds>(function_time).count() << " ms" << std::endl;
}

int main() {
    std::cout << "STRATEGY AND POLICY-BASED DESIGN\n";
    std::cout << "================================\n";
    
    demonstrate_classic_strategy();
    demonstrate_function_strategy();
    demonstrate_policy_based_design();
    demonstrate_template_strategy();
    demonstrate_policy_composition();
    performance_comparison();
    
    std::cout << "\nDesign Comparison:" << std::endl;
    std::cout << "Strategy Pattern: Runtime flexibility, virtual function overhead" << std::endl;
    std::cout << "Policy-based: Compile-time selection, zero overhead" << std::endl;
    std::cout << "Function-based: Modern C++, std::function flexibility" << std::endl;
    std::cout << "Template Strategy: Best performance, compile-time polymorphism" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Strategy pattern enables algorithm selection at runtime
- Policy-based design provides compile-time flexibility
- Template strategies offer best performance through inlining
- Function-based strategies use std::function for flexibility
- Policy composition enables orthogonal feature combinations
- Virtual functions have runtime overhead vs templates
- Choose based on requirements: runtime vs compile-time selection
- Modern C++ favors templates and lambdas over inheritance
*/
