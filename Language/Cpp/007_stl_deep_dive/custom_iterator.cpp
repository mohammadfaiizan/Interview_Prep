/*
 * STL DEEP DIVE - CUSTOM ITERATOR IMPLEMENTATION
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra custom_iterator.cpp -o custom_iterator
 */

#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. SIMPLE FORWARD ITERATOR
// =============================================================================

template<typename T>
class SimpleForwardIterator {
private:
    T* ptr_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    explicit SimpleForwardIterator(T* ptr = nullptr) : ptr_(ptr) {}
    
    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }
    
    SimpleForwardIterator& operator++() { ++ptr_; return *this; }
    SimpleForwardIterator operator++(int) { auto tmp = *this; ++ptr_; return tmp; }
    
    bool operator==(const SimpleForwardIterator& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const SimpleForwardIterator& other) const { return ptr_ != other.ptr_; }
};

void demonstrate_forward_iterator() {
    SECTION("Simple Forward Iterator");
    
    int data[] = {1, 2, 3, 4, 5};
    SimpleForwardIterator<int> begin(data);
    SimpleForwardIterator<int> end(data + 5);
    
    std::cout << "Forward iteration: ";
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Works with STL algorithms
    int sum = std::accumulate(begin, end, 0);
    std::cout << "Sum: " << sum << std::endl;
}

// =============================================================================
// 2. RANGE-BASED ITERATOR
// =============================================================================

template<typename T>
class RangeIterator {
private:
    T current_;
    T step_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    RangeIterator(T start, T step = T{1}) : current_(start), step_(step) {}
    
    T operator*() const { return current_; }
    
    RangeIterator& operator++() { current_ += step_; return *this; }
    RangeIterator operator++(int) { auto tmp = *this; current_ += step_; return tmp; }
    
    bool operator==(const RangeIterator& other) const { return current_ == other.current_; }
    bool operator!=(const RangeIterator& other) const { return current_ != other.current_; }
};

template<typename T>
class Range {
private:
    T start_, end_, step_;
    
public:
    Range(T start, T end, T step = T{1}) : start_(start), end_(end), step_(step) {}
    
    RangeIterator<T> begin() const { return RangeIterator<T>(start_, step_); }
    RangeIterator<T> end() const { return RangeIterator<T>(end_, step_); }
};

void demonstrate_range_iterator() {
    SECTION("Range-based Iterator");
    
    std::cout << "Range 1 to 10: ";
    for (int x : Range<int>(1, 11)) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Range 0 to 20 step 3: ";
    for (int x : Range<int>(0, 21, 3)) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 3. FILTER ITERATOR
// =============================================================================

template<typename Iterator, typename Predicate>
class FilterIterator {
private:
    Iterator current_;
    Iterator end_;
    Predicate pred_;
    
    void advance_to_valid() {
        while (current_ != end_ && !pred_(*current_)) {
            ++current_;
        }
    }
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using reference = typename std::iterator_traits<Iterator>::reference;
    
    FilterIterator(Iterator current, Iterator end, Predicate pred)
        : current_(current), end_(end), pred_(pred) {
        advance_to_valid();
    }
    
    reference operator*() const { return *current_; }
    pointer operator->() const { return current_.operator->(); }
    
    FilterIterator& operator++() {
        ++current_;
        advance_to_valid();
        return *this;
    }
    
    FilterIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const FilterIterator& other) const { return current_ == other.current_; }
    bool operator!=(const FilterIterator& other) const { return current_ != other.current_; }
};

template<typename Iterator, typename Predicate>
FilterIterator<Iterator, Predicate> make_filter_iterator(Iterator it, Iterator end, Predicate pred) {
    return FilterIterator<Iterator, Predicate>(it, end, pred);
}

void demonstrate_filter_iterator() {
    SECTION("Filter Iterator");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto is_even = [](int x) { return x % 2 == 0; };
    auto begin = make_filter_iterator(data.begin(), data.end(), is_even);
    auto end = make_filter_iterator(data.end(), data.end(), is_even);
    
    std::cout << "Even numbers only: ";
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 4. TRANSFORM ITERATOR
// =============================================================================

template<typename Iterator, typename Function>
class TransformIterator {
private:
    Iterator iter_;
    Function func_;
    
public:
    using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
    using value_type = decltype(func_(*iter_));
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using pointer = value_type*;
    using reference = value_type;
    
    TransformIterator(Iterator iter, Function func) : iter_(iter), func_(func) {}
    
    value_type operator*() const { return func_(*iter_); }
    
    TransformIterator& operator++() { ++iter_; return *this; }
    TransformIterator operator++(int) { auto tmp = *this; ++iter_; return tmp; }
    
    bool operator==(const TransformIterator& other) const { return iter_ == other.iter_; }
    bool operator!=(const TransformIterator& other) const { return iter_ != other.iter_; }
};

template<typename Iterator, typename Function>
TransformIterator<Iterator, Function> make_transform_iterator(Iterator it, Function func) {
    return TransformIterator<Iterator, Function>(it, func);
}

void demonstrate_transform_iterator() {
    SECTION("Transform Iterator");
    
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    auto square = [](int x) { return x * x; };
    auto begin = make_transform_iterator(data.begin(), square);
    auto end = make_transform_iterator(data.end(), square);
    
    std::cout << "Squared values: ";
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Works with algorithms
    int sum_of_squares = std::accumulate(begin, end, 0);
    std::cout << "Sum of squares: " << sum_of_squares << std::endl;
}

// =============================================================================
// 5. CIRCULAR ITERATOR
// =============================================================================

template<typename Iterator>
class CircularIterator {
private:
    Iterator current_;
    Iterator begin_;
    Iterator end_;
    mutable size_t count_;
    size_t max_count_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using reference = typename std::iterator_traits<Iterator>::reference;
    
    CircularIterator(Iterator begin, Iterator end, size_t max_iterations = 100)
        : current_(begin), begin_(begin), end_(end), count_(0), max_count_(max_iterations) {}
    
    reference operator*() const { return *current_; }
    pointer operator->() const { return current_.operator->(); }
    
    CircularIterator& operator++() {
        ++current_;
        ++count_;
        if (current_ == end_) {
            current_ = begin_;
        }
        return *this;
    }
    
    bool operator==(const CircularIterator& other) const { 
        return count_ == other.count_ && current_ == other.current_; 
    }
    bool operator!=(const CircularIterator& other) const { 
        return count_ < max_count_; 
    }
};

void demonstrate_circular_iterator() {
    SECTION("Circular Iterator");
    
    std::vector<int> data = {1, 2, 3};
    CircularIterator<std::vector<int>::iterator> begin(data.begin(), data.end(), 10);
    CircularIterator<std::vector<int>::iterator> end(data.end(), data.end(), 10);
    
    std::cout << "Circular iteration (10 times): ";
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "CUSTOM ITERATOR IMPLEMENTATION\n";
    std::cout << "==============================\n";
    
    demonstrate_forward_iterator();
    demonstrate_range_iterator();
    demonstrate_filter_iterator();
    demonstrate_transform_iterator();
    demonstrate_circular_iterator();
    
    std::cout << "\nKey Benefits of Custom Iterators:" << std::endl;
    std::cout << "- Lazy evaluation" << std::endl;
    std::cout << "- Memory efficiency" << std::endl;
    std::cout << "- Composability" << std::endl;
    std::cout << "- STL algorithm compatibility" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Custom iterators must define required type aliases
- Iterator categories determine available operations
- Filter iterators skip elements based on predicate
- Transform iterators modify values on-the-fly
- Circular iterators wrap around container boundaries
- Lazy evaluation improves performance and memory usage
- Custom iterators work seamlessly with STL algorithms
- Proper iterator design enables powerful abstractions
*/
