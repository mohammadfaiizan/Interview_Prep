/*
 * STL DEEP DIVE - ITERATOR CATEGORIES
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra iterator_categories.cpp -o iterator_categories
 */

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. ITERATOR CATEGORIES OVERVIEW
// =============================================================================

void demonstrate_iterator_categories() {
    SECTION("Iterator Categories Overview");
    
    std::cout << "Iterator Categories Hierarchy:" << std::endl;
    std::cout << "1. Input Iterator (read once, forward only)" << std::endl;
    std::cout << "2. Output Iterator (write once, forward only)" << std::endl;
    std::cout << "3. Forward Iterator (read/write multiple times, forward)" << std::endl;
    std::cout << "4. Bidirectional Iterator (forward + backward)" << std::endl;
    std::cout << "5. Random Access Iterator (jump to any position)" << std::endl;
    std::cout << "6. Contiguous Iterator (C++20, guaranteed contiguous memory)" << std::endl;
}

// =============================================================================
// 2. INPUT/OUTPUT ITERATORS
// =============================================================================

void demonstrate_input_output_iterators() {
    SECTION("Input/Output Iterators");
    
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    // Input iterator example (istream_iterator)
    std::cout << "Input Iterator (conceptual):" << std::endl;
    std::cout << "- Read only, single pass" << std::endl;
    std::cout << "- ++it, *it, it != other" << std::endl;
    
    // Output iterator example (ostream_iterator)
    std::cout << "\nOutput Iterator example:" << std::endl;
    std::copy(data.begin(), data.end(), 
              std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    
    std::cout << "Output Iterator properties:" << std::endl;
    std::cout << "- Write only, single pass" << std::endl;
    std::cout << "- ++it, *it = value" << std::endl;
}

// =============================================================================
// 3. FORWARD ITERATORS
// =============================================================================

void demonstrate_forward_iterators() {
    SECTION("Forward Iterators");
    
    std::forward_list<int> flist = {1, 2, 3, 4, 5};
    
    std::cout << "Forward Iterator capabilities:" << std::endl;
    std::cout << "- Multi-pass (can iterate multiple times)" << std::endl;
    std::cout << "- Read/write access" << std::endl;
    std::cout << "- Forward movement only" << std::endl;
    
    // Find and modify using forward iterator
    auto it = std::find(flist.begin(), flist.end(), 3);
    if (it != flist.end()) {
        *it = 30;
        std::cout << "Modified element 3 to 30" << std::endl;
    }
    
    std::cout << "Forward list contents: ";
    for (int value : flist) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 4. BIDIRECTIONAL ITERATORS
// =============================================================================

void demonstrate_bidirectional_iterators() {
    SECTION("Bidirectional Iterators");
    
    std::list<int> lst = {1, 2, 3, 4, 5};
    std::set<int> s = {1, 2, 3, 4, 5};
    
    std::cout << "Bidirectional Iterator capabilities:" << std::endl;
    std::cout << "- All forward iterator operations" << std::endl;
    std::cout << "- Backward movement (--it)" << std::endl;
    
    // Demonstrate reverse iteration
    std::cout << "List forward: ";
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    std::cout << "List backward: ";
    for (auto it = lst.rbegin(); it != lst.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Set iterators are also bidirectional
    std::cout << "Set reverse: ";
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. RANDOM ACCESS ITERATORS
// =============================================================================

void demonstrate_random_access_iterators() {
    SECTION("Random Access Iterators");
    
    std::vector<int> vec = {10, 20, 30, 40, 50};
    
    std::cout << "Random Access Iterator capabilities:" << std::endl;
    std::cout << "- All bidirectional iterator operations" << std::endl;
    std::cout << "- Jump to any position (it + n, it - n)" << std::endl;
    std::cout << "- Comparison operators (<, >, <=, >=)" << std::endl;
    std::cout << "- Subscript operator (it[n])" << std::endl;
    
    auto it = vec.begin();
    
    // Random access operations
    std::cout << "Element at position 0: " << *it << std::endl;
    std::cout << "Element at position 2: " << *(it + 2) << std::endl;
    std::cout << "Element at position 4: " << it[4] << std::endl;
    
    // Iterator arithmetic
    auto end_it = vec.end();
    std::cout << "Distance from begin to end: " << (end_it - it) << std::endl;
    
    // Efficient algorithms possible with random access
    std::sort(vec.begin(), vec.end(), std::greater<int>());
    std::cout << "Sorted (descending): ";
    for (int value : vec) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 6. ITERATOR TRAITS AND TAG DISPATCHING
// =============================================================================

template<typename Iterator>
void advance_impl(Iterator& it, int n, std::input_iterator_tag) {
    std::cout << "Using input iterator advance (slow)" << std::endl;
    while (n > 0) {
        ++it;
        --n;
    }
}

template<typename Iterator>
void advance_impl(Iterator& it, int n, std::bidirectional_iterator_tag) {
    std::cout << "Using bidirectional iterator advance (medium)" << std::endl;
    if (n > 0) {
        while (n > 0) { ++it; --n; }
    } else {
        while (n < 0) { --it; ++n; }
    }
}

template<typename Iterator>
void advance_impl(Iterator& it, int n, std::random_access_iterator_tag) {
    std::cout << "Using random access iterator advance (fast)" << std::endl;
    it += n;
}

template<typename Iterator>
void my_advance(Iterator& it, int n) {
    advance_impl(it, n, typename std::iterator_traits<Iterator>::iterator_category{});
}

void demonstrate_iterator_traits() {
    SECTION("Iterator Traits and Tag Dispatching");
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {1, 2, 3, 4, 5};
    
    // Test with vector (random access)
    auto vec_it = vec.begin();
    std::cout << "Advancing vector iterator by 3:" << std::endl;
    my_advance(vec_it, 3);
    std::cout << "Value: " << *vec_it << std::endl;
    
    // Test with list (bidirectional)
    auto list_it = lst.begin();
    std::cout << "\nAdvancing list iterator by 2:" << std::endl;
    my_advance(list_it, 2);
    std::cout << "Value: " << *list_it << std::endl;
    
    std::cout << "\nIterator traits provide compile-time information:" << std::endl;
    std::cout << "- iterator_category" << std::endl;
    std::cout << "- value_type" << std::endl;
    std::cout << "- difference_type" << std::endl;
    std::cout << "- pointer" << std::endl;
    std::cout << "- reference" << std::endl;
}

// =============================================================================
// 7. CUSTOM ITERATOR EXAMPLE
// =============================================================================

template<typename T>
class simple_vector_iterator {
private:
    T* ptr_;
    
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    explicit simple_vector_iterator(T* ptr) : ptr_(ptr) {}
    
    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }
    
    simple_vector_iterator& operator++() { ++ptr_; return *this; }
    simple_vector_iterator operator++(int) { auto tmp = *this; ++ptr_; return tmp; }
    
    simple_vector_iterator& operator--() { --ptr_; return *this; }
    simple_vector_iterator operator--(int) { auto tmp = *this; --ptr_; return tmp; }
    
    simple_vector_iterator& operator+=(difference_type n) { ptr_ += n; return *this; }
    simple_vector_iterator& operator-=(difference_type n) { ptr_ -= n; return *this; }
    
    simple_vector_iterator operator+(difference_type n) const { return simple_vector_iterator(ptr_ + n); }
    simple_vector_iterator operator-(difference_type n) const { return simple_vector_iterator(ptr_ - n); }
    
    difference_type operator-(const simple_vector_iterator& other) const { return ptr_ - other.ptr_; }
    
    reference operator[](difference_type n) const { return ptr_[n]; }
    
    bool operator==(const simple_vector_iterator& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const simple_vector_iterator& other) const { return ptr_ != other.ptr_; }
    bool operator<(const simple_vector_iterator& other) const { return ptr_ < other.ptr_; }
    bool operator>(const simple_vector_iterator& other) const { return ptr_ > other.ptr_; }
    bool operator<=(const simple_vector_iterator& other) const { return ptr_ <= other.ptr_; }
    bool operator>=(const simple_vector_iterator& other) const { return ptr_ >= other.ptr_; }
};

void demonstrate_custom_iterator() {
    SECTION("Custom Iterator Example");
    
    int data[] = {1, 2, 3, 4, 5};
    simple_vector_iterator<int> begin(data);
    simple_vector_iterator<int> end(data + 5);
    
    std::cout << "Custom iterator usage:" << std::endl;
    std::cout << "Forward iteration: ";
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Random access: " << begin[2] << std::endl;
    std::cout << "Distance: " << (end - begin) << std::endl;
    
    // Works with STL algorithms
    std::cout << "Sum using std::accumulate: " 
              << std::accumulate(begin, end, 0) << std::endl;
}

int main() {
    std::cout << "STL ITERATOR CATEGORIES TUTORIAL\n";
    std::cout << "================================\n";
    
    demonstrate_iterator_categories();
    demonstrate_input_output_iterators();
    demonstrate_forward_iterators();
    demonstrate_bidirectional_iterators();
    demonstrate_random_access_iterators();
    demonstrate_iterator_traits();
    demonstrate_custom_iterator();
    
    return 0;
}

/*
KEY CONCEPTS:
- Iterator categories form a hierarchy of capabilities
- Input/Output: single-pass, read-only/write-only
- Forward: multi-pass, read/write, forward only
- Bidirectional: adds backward movement
- Random Access: adds jumping and comparison
- Iterator traits enable compile-time optimization
- Tag dispatching selects optimal algorithm implementation
- Custom iterators must satisfy category requirements
*/
