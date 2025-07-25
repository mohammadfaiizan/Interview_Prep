/*
 * STL DEEP DIVE - ALGORITHMS INTERNAL IMPLEMENTATION
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -O2 algorithms_internal.cpp -o algorithms_internal
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <random>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. SORTING ALGORITHMS INTERNALS
// =============================================================================

template<typename RandomIt>
void my_quicksort(RandomIt first, RandomIt last) {
    if (first >= last) return;
    
    auto pivot = std::partition(first, last, 
        [pivot_val = *first](const auto& elem) {
            return elem < pivot_val;
        });
    
    std::iter_swap(first, pivot - 1);
    my_quicksort(first, pivot - 1);
    my_quicksort(pivot, last);
}

template<typename RandomIt>
void my_heapsort(RandomIt first, RandomIt last) {
    std::make_heap(first, last);
    std::sort_heap(first, last);
}

template<typename RandomIt>
void my_introsort(RandomIt first, RandomIt last, int depth_limit) {
    if (last - first < 16) {
        // Use insertion sort for small ranges
        for (auto it = first + 1; it != last; ++it) {
            auto key = *it;
            auto pos = it;
            while (pos != first && *(pos - 1) > key) {
                *pos = *(pos - 1);
                --pos;
            }
            *pos = key;
        }
        return;
    }
    
    if (depth_limit == 0) {
        // Switch to heapsort when recursion depth is too deep
        my_heapsort(first, last);
        return;
    }
    
    // Partition and recurse
    auto pivot = std::partition(first, last,
        [pivot_val = *first](const auto& elem) {
            return elem < pivot_val;
        });
    
    std::iter_swap(first, pivot - 1);
    my_introsort(first, pivot - 1, depth_limit - 1);
    my_introsort(pivot, last, depth_limit - 1);
}

void demonstrate_sorting_internals() {
    SECTION("Sorting Algorithms Internals");
    
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90, 5, 77, 30};
    std::cout << "Original: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Test custom quicksort
    auto data1 = data;
    my_quicksort(data1.begin(), data1.end());
    std::cout << "Quicksort: ";
    for (int x : data1) std::cout << x << " ";
    std::cout << std::endl;
    
    // Test custom heapsort
    auto data2 = data;
    my_heapsort(data2.begin(), data2.end());
    std::cout << "Heapsort: ";
    for (int x : data2) std::cout << x << " ";
    std::cout << std::endl;
    
    // Test custom introsort
    auto data3 = data;
    int depth_limit = 2 * std::log2(data3.size());
    my_introsort(data3.begin(), data3.end(), depth_limit);
    std::cout << "Introsort: ";
    for (int x : data3) std::cout << x << " ";
    std::cout << std::endl;
    
    std::cout << "\nstd::sort typically uses introsort (intro + heap + insertion)" << std::endl;
}

// =============================================================================
// 2. SEARCH ALGORITHMS INTERNALS
// =============================================================================

template<typename ForwardIt, typename T>
ForwardIt my_lower_bound(ForwardIt first, ForwardIt last, const T& value) {
    auto count = std::distance(first, last);
    
    while (count > 0) {
        auto step = count / 2;
        auto it = first;
        std::advance(it, step);
        
        if (*it < value) {
            first = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}

template<typename ForwardIt, typename T>
bool my_binary_search(ForwardIt first, ForwardIt last, const T& value) {
    auto it = my_lower_bound(first, last, value);
    return it != last && !(value < *it);
}

template<typename InputIt, typename T>
InputIt my_find(InputIt first, InputIt last, const T& value) {
    while (first != last && *first != value) {
        ++first;
    }
    return first;
}

void demonstrate_search_internals() {
    SECTION("Search Algorithms Internals");
    
    std::vector<int> sorted_data = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    std::cout << "Sorted data: ";
    for (int x : sorted_data) std::cout << x << " ";
    std::cout << std::endl;
    
    int target = 7;
    
    // Binary search
    bool found = my_binary_search(sorted_data.begin(), sorted_data.end(), target);
    std::cout << "Binary search for " << target << ": " << (found ? "found" : "not found") << std::endl;
    
    // Lower bound
    auto lb = my_lower_bound(sorted_data.begin(), sorted_data.end(), target);
    std::cout << "Lower bound position: " << std::distance(sorted_data.begin(), lb) << std::endl;
    
    // Linear search
    std::vector<int> unsorted_data = {19, 3, 15, 7, 1, 9, 13, 5, 17, 11};
    auto found_it = my_find(unsorted_data.begin(), unsorted_data.end(), target);
    if (found_it != unsorted_data.end()) {
        std::cout << "Linear search found " << target << " at position " 
                  << std::distance(unsorted_data.begin(), found_it) << std::endl;
    }
}

// =============================================================================
// 3. HEAP ALGORITHMS INTERNALS
// =============================================================================

template<typename RandomIt>
void my_push_heap(RandomIt first, RandomIt last) {
    if (first == last) return;
    
    auto child = last - 1;
    auto parent_idx = (std::distance(first, child) - 1) / 2;
    auto parent = first + parent_idx;
    
    while (child != first && *parent < *child) {
        std::iter_swap(parent, child);
        child = parent;
        if (child == first) break;
        parent_idx = (std::distance(first, child) - 1) / 2;
        parent = first + parent_idx;
    }
}

template<typename RandomIt>
void my_pop_heap(RandomIt first, RandomIt last) {
    if (first == last) return;
    
    --last;
    std::iter_swap(first, last);
    
    // Heapify down
    auto parent = first;
    auto heap_size = std::distance(first, last);
    
    while (true) {
        auto parent_idx = std::distance(first, parent);
        auto left_child_idx = 2 * parent_idx + 1;
        auto right_child_idx = 2 * parent_idx + 2;
        auto largest_idx = parent_idx;
        
        if (left_child_idx < heap_size && 
            *(first + left_child_idx) > *(first + largest_idx)) {
            largest_idx = left_child_idx;
        }
        
        if (right_child_idx < heap_size && 
            *(first + right_child_idx) > *(first + largest_idx)) {
            largest_idx = right_child_idx;
        }
        
        if (largest_idx == parent_idx) break;
        
        std::iter_swap(first + parent_idx, first + largest_idx);
        parent = first + largest_idx;
    }
}

void demonstrate_heap_internals() {
    SECTION("Heap Algorithms Internals");
    
    std::vector<int> data = {4, 1, 3, 2, 16, 9, 10, 14, 8, 7};
    std::cout << "Original: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Make heap
    std::make_heap(data.begin(), data.end());
    std::cout << "After make_heap: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Push new element
    data.push_back(15);
    my_push_heap(data.begin(), data.end());
    std::cout << "After push_heap(15): ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Pop max element
    my_pop_heap(data.begin(), data.end());
    std::cout << "After pop_heap: ";
    for (int x : data) std::cout << x << " ";
    std::cout << " (popped: " << data.back() << ")" << std::endl;
}

// =============================================================================
// 4. PARTITION ALGORITHMS INTERNALS
// =============================================================================

template<typename ForwardIt, typename UnaryPredicate>
ForwardIt my_partition(ForwardIt first, ForwardIt last, UnaryPredicate pred) {
    first = std::find_if_not(first, last, pred);
    if (first == last) return first;
    
    for (auto it = std::next(first); it != last; ++it) {
        if (pred(*it)) {
            std::iter_swap(it, first);
            ++first;
        }
    }
    return first;
}

template<typename ForwardIt, typename UnaryPredicate>
ForwardIt my_stable_partition(ForwardIt first, ForwardIt last, UnaryPredicate pred) {
    auto true_part = first;
    
    for (auto it = first; it != last; ++it) {
        if (pred(*it)) {
            if (it != true_part) {
                // Rotate to maintain stability
                auto temp = *it;
                std::move_backward(true_part, it, it + 1);
                *true_part = temp;
            }
            ++true_part;
        }
    }
    return true_part;
}

void demonstrate_partition_internals() {
    SECTION("Partition Algorithms Internals");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::cout << "Original: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    // Partition even/odd
    auto data1 = data;
    auto partition_point = my_partition(data1.begin(), data1.end(), 
                                       [](int x) { return x % 2 == 0; });
    std::cout << "Partitioned (even first): ";
    for (int x : data1) std::cout << x << " ";
    std::cout << " | partition at position " << std::distance(data1.begin(), partition_point) << std::endl;
    
    // Stable partition
    auto data2 = data;
    auto stable_point = my_stable_partition(data2.begin(), data2.end(),
                                           [](int x) { return x % 2 == 0; });
    std::cout << "Stable partition: ";
    for (int x : data2) std::cout << x << " ";
    std::cout << " | partition at position " << std::distance(data2.begin(), stable_point) << std::endl;
}

// =============================================================================
// 5. ALGORITHM COMPLEXITY ANALYSIS
// =============================================================================

void analyze_algorithm_complexity() {
    SECTION("Algorithm Complexity Analysis");
    
    std::cout << "TIME COMPLEXITIES:" << std::endl;
    std::cout << "Sort algorithms:" << std::endl;
    std::cout << "  std::sort: O(n log n) average, O(n²) worst (introsort)" << std::endl;
    std::cout << "  std::stable_sort: O(n log n) guaranteed (merge sort)" << std::endl;
    std::cout << "  std::partial_sort: O(n log k) where k is partial size" << std::endl;
    
    std::cout << "\nSearch algorithms:" << std::endl;
    std::cout << "  std::find: O(n) linear search" << std::endl;
    std::cout << "  std::binary_search: O(log n) on sorted range" << std::endl;
    std::cout << "  std::lower_bound: O(log n) on sorted range" << std::endl;
    
    std::cout << "\nHeap algorithms:" << std::endl;
    std::cout << "  std::make_heap: O(n)" << std::endl;
    std::cout << "  std::push_heap: O(log n)" << std::endl;
    std::cout << "  std::pop_heap: O(log n)" << std::endl;
    
    std::cout << "\nPartition algorithms:" << std::endl;
    std::cout << "  std::partition: O(n)" << std::endl;
    std::cout << "  std::stable_partition: O(n log n) worst, O(n) if memory available" << std::endl;
    
    std::cout << "\nSPACE COMPLEXITIES:" << std::endl;
    std::cout << "  Most algorithms: O(1) additional space" << std::endl;
    std::cout << "  std::stable_sort: O(n) additional space" << std::endl;
    std::cout << "  std::stable_partition: O(n) additional space (worst case)" << std::endl;
}

// =============================================================================
// 6. PERFORMANCE BENCHMARKS
// =============================================================================

void benchmark_algorithms() {
    SECTION("Algorithm Performance Benchmarks");
    
    const size_t size = 100000;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Generate test data
    std::vector<int> random_data(size);
    std::uniform_int_distribution<> dis(1, 1000000);
    std::generate(random_data.begin(), random_data.end(), [&]() { return dis(gen); });
    
    // Sort benchmarks
    auto data1 = random_data;
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data1.begin(), data1.end());
    auto sort_time = std::chrono::high_resolution_clock::now() - start;
    
    auto data2 = random_data;
    start = std::chrono::high_resolution_clock::now();
    std::stable_sort(data2.begin(), data2.end());
    auto stable_sort_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Sort " << size << " elements:" << std::endl;
    std::cout << "  std::sort: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(sort_time).count() 
              << " ms" << std::endl;
    std::cout << "  std::stable_sort: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(stable_sort_time).count() 
              << " ms" << std::endl;
    
    // Search benchmarks
    int target = data1[size/2];
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        std::binary_search(data1.begin(), data1.end(), target);
    }
    auto binary_search_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        std::find(random_data.begin(), random_data.end(), target);
    }
    auto linear_search_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "\nSearch 1000 times:" << std::endl;
    std::cout << "  Binary search: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(binary_search_time).count() 
              << " μs" << std::endl;
    std::cout << "  Linear search: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(linear_search_time).count() 
              << " μs" << std::endl;
}

int main() {
    std::cout << "STL ALGORITHMS INTERNAL IMPLEMENTATION\n";
    std::cout << "=====================================\n";
    
    demonstrate_sorting_internals();
    demonstrate_search_internals();
    demonstrate_heap_internals();
    demonstrate_partition_internals();
    analyze_algorithm_complexity();
    benchmark_algorithms();
    
    return 0;
}

/*
KEY INSIGHTS:
- std::sort uses introsort (quicksort + heapsort + insertion sort)
- Binary search requires sorted data, O(log n) vs O(n) linear
- Heap operations maintain heap property through sift up/down
- Partition is fundamental to quicksort and selection algorithms
- Stable algorithms preserve relative order of equal elements
- Algorithm choice depends on data characteristics and requirements
- Understanding internals helps optimize algorithm selection
- Complexity analysis guides performance expectations
*/
