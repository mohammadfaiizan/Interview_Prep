/*
 * STL DEEP DIVE - CONTAINERS PERFORMANCE ANALYSIS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -O2 stl_containers_performance.cpp -o stl_performance
 */

#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <array>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <chrono>
#include <random>
#include <algorithm>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl
#define MEASURE_TIME(code) do { \
    auto start = std::chrono::high_resolution_clock::now(); \
    code; \
    auto end = std::chrono::high_resolution_clock::now(); \
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); \
    std::cout << "Time: " << duration.count() << " μs" << std::endl; \
} while(0)

// =============================================================================
// 1. SEQUENCE CONTAINERS PERFORMANCE
// =============================================================================

void test_sequence_containers() {
    SECTION("Sequence Containers Performance");
    
    const size_t size = 100000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    // Vector performance
    std::cout << "Vector operations:" << std::endl;
    std::vector<int> vec;
    vec.reserve(size);
    
    std::cout << "Push back: ";
    MEASURE_TIME({
        for (size_t i = 0; i < size; ++i) {
            vec.push_back(dis(gen));
        }
    });
    
    std::cout << "Random access: ";
    MEASURE_TIME({
        volatile int sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += vec[i % vec.size()];
        }
    });
    
    std::cout << "Insert middle: ";
    MEASURE_TIME({
        for (int i = 0; i < 1000; ++i) {
            vec.insert(vec.begin() + vec.size()/2, dis(gen));
        }
    });
    
    // Deque performance
    std::cout << "\nDeque operations:" << std::endl;
    std::deque<int> deq;
    
    std::cout << "Push back: ";
    MEASURE_TIME({
        for (size_t i = 0; i < size; ++i) {
            deq.push_back(dis(gen));
        }
    });
    
    std::cout << "Push front: ";
    MEASURE_TIME({
        for (int i = 0; i < 10000; ++i) {
            deq.push_front(dis(gen));
        }
    });
    
    // List performance
    std::cout << "\nList operations:" << std::endl;
    std::list<int> lst;
    
    std::cout << "Push back: ";
    MEASURE_TIME({
        for (size_t i = 0; i < size; ++i) {
            lst.push_back(dis(gen));
        }
    });
    
    std::cout << "Insert middle: ";
    MEASURE_TIME({
        auto it = lst.begin();
        std::advance(it, lst.size()/2);
        for (int i = 0; i < 1000; ++i) {
            lst.insert(it, dis(gen));
        }
    });
}

// =============================================================================
// 2. ASSOCIATIVE CONTAINERS PERFORMANCE
// =============================================================================

void test_associative_containers() {
    SECTION("Associative Containers Performance");
    
    const size_t size = 100000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    std::vector<int> test_data;
    for (size_t i = 0; i < size; ++i) {
        test_data.push_back(dis(gen));
    }
    
    // Set performance
    std::cout << "Set operations:" << std::endl;
    std::set<int> s;
    
    std::cout << "Insert: ";
    MEASURE_TIME({
        for (int value : test_data) {
            s.insert(value);
        }
    });
    
    std::cout << "Find: ";
    MEASURE_TIME({
        for (int i = 0; i < 10000; ++i) {
            s.find(dis(gen));
        }
    });
    
    // Unordered set performance
    std::cout << "\nUnordered set operations:" << std::endl;
    std::unordered_set<int> us;
    
    std::cout << "Insert: ";
    MEASURE_TIME({
        for (int value : test_data) {
            us.insert(value);
        }
    });
    
    std::cout << "Find: ";
    MEASURE_TIME({
        for (int i = 0; i < 10000; ++i) {
            us.find(dis(gen));
        }
    });
    
    // Map performance
    std::cout << "\nMap operations:" << std::endl;
    std::map<int, int> m;
    
    std::cout << "Insert: ";
    MEASURE_TIME({
        for (int value : test_data) {
            m[value] = value * 2;
        }
    });
    
    // Unordered map performance
    std::cout << "\nUnordered map operations:" << std::endl;
    std::unordered_map<int, int> um;
    
    std::cout << "Insert: ";
    MEASURE_TIME({
        for (int value : test_data) {
            um[value] = value * 2;
        }
    });
}

// =============================================================================
// 3. MEMORY USAGE ANALYSIS
// =============================================================================

void analyze_memory_usage() {
    SECTION("Memory Usage Analysis");
    
    const size_t count = 1000;
    
    std::cout << "Memory overhead per element (approximate):" << std::endl;
    
    // Vector
    std::vector<int> vec(count, 42);
    size_t vec_size = sizeof(vec) + vec.capacity() * sizeof(int);
    std::cout << "Vector: " << static_cast<double>(vec_size) / count << " bytes/element" << std::endl;
    
    // List
    std::list<int> lst(count, 42);
    size_t list_node_size = sizeof(int) + 2 * sizeof(void*); // data + 2 pointers
    std::cout << "List: ~" << list_node_size << " bytes/element (node overhead)" << std::endl;
    
    // Set
    std::set<int> s;
    for (int i = 0; i < static_cast<int>(count); ++i) s.insert(i);
    size_t set_node_size = sizeof(int) + 3 * sizeof(void*) + sizeof(bool); // RB tree node
    std::cout << "Set: ~" << set_node_size << " bytes/element (tree overhead)" << std::endl;
    
    // Unordered set
    std::unordered_set<int> us;
    for (int i = 0; i < static_cast<int>(count); ++i) us.insert(i);
    std::cout << "Unordered set: ~" << sizeof(int) + sizeof(void*) << " bytes/element + hash table overhead" << std::endl;
}

// =============================================================================
// 4. CACHE PERFORMANCE
// =============================================================================

void test_cache_performance() {
    SECTION("Cache Performance");
    
    const size_t size = 1000000;
    
    // Vector (cache-friendly)
    std::vector<int> vec(size);
    std::iota(vec.begin(), vec.end(), 1);
    
    std::cout << "Vector sequential access: ";
    MEASURE_TIME({
        volatile long sum = 0;
        for (int value : vec) {
            sum += value;
        }
    });
    
    // List (cache-unfriendly)
    std::list<int> lst(size);
    std::iota(lst.begin(), lst.end(), 1);
    
    std::cout << "List sequential access: ";
    MEASURE_TIME({
        volatile long sum = 0;
        for (int value : lst) {
            sum += value;
        }
    });
    
    // Random access comparison
    std::vector<size_t> indices(size);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), std::mt19937{});
    
    std::cout << "Vector random access: ";
    MEASURE_TIME({
        volatile long sum = 0;
        for (size_t idx : indices) {
            sum += vec[idx];
        }
    });
}

// =============================================================================
// 5. ALGORITHM PERFORMANCE
// =============================================================================

void test_algorithm_performance() {
    SECTION("Algorithm Performance");
    
    const size_t size = 100000;
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
    
    // Sort performance
    auto data_copy = data;
    std::cout << "std::sort: ";
    MEASURE_TIME({
        std::sort(data_copy.begin(), data_copy.end());
    });
    
    // Binary search vs linear search
    int target = data_copy[size/2];
    
    std::cout << "Binary search (sorted): ";
    MEASURE_TIME({
        for (int i = 0; i < 1000; ++i) {
            std::binary_search(data_copy.begin(), data_copy.end(), target);
        }
    });
    
    std::cout << "Linear search (unsorted): ";
    MEASURE_TIME({
        for (int i = 0; i < 1000; ++i) {
            std::find(data.begin(), data.end(), target);
        }
    });
}

// =============================================================================
// 6. CONTAINER SELECTION GUIDE
// =============================================================================

void container_selection_guide() {
    SECTION("Container Selection Guide");
    
    std::cout << "SEQUENCE CONTAINERS:" << std::endl;
    std::cout << "vector: Fast random access, cache-friendly, dynamic size" << std::endl;
    std::cout << "  Use for: General purpose, frequent random access" << std::endl;
    std::cout << "  Avoid: Frequent insertions in middle" << std::endl;
    
    std::cout << "\ndeque: Fast front/back operations, good random access" << std::endl;
    std::cout << "  Use for: Queue operations, sliding window" << std::endl;
    std::cout << "  Avoid: Memory-constrained environments" << std::endl;
    
    std::cout << "\nlist: Fast insertion/deletion anywhere, stable iterators" << std::endl;
    std::cout << "  Use for: Frequent insertions/deletions, splice operations" << std::endl;
    std::cout << "  Avoid: Random access, cache-sensitive code" << std::endl;
    
    std::cout << "\nASSOCIATIVE CONTAINERS:" << std::endl;
    std::cout << "set/map: Ordered, O(log n) operations, stable iteration order" << std::endl;
    std::cout << "  Use for: Sorted data, range queries" << std::endl;
    std::cout << "  Avoid: Frequent lookups, simple key types" << std::endl;
    
    std::cout << "\nunordered_set/map: O(1) average operations, hash-based" << std::endl;
    std::cout << "  Use for: Fast lookups, simple keys" << std::endl;
    std::cout << "  Avoid: Ordered iteration, worst-case guarantees" << std::endl;
}

// =============================================================================
// 7. CUSTOM PERFORMANCE TESTS
// =============================================================================

template<typename Container>
void benchmark_container(const std::string& name, size_t size) {
    Container container;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    std::cout << name << " benchmark:" << std::endl;
    
    // Insert performance
    std::cout << "  Insert " << size << " elements: ";
    MEASURE_TIME({
        for (size_t i = 0; i < size; ++i) {
            if constexpr (std::is_same_v<Container, std::vector<int>> ||
                         std::is_same_v<Container, std::deque<int>> ||
                         std::is_same_v<Container, std::list<int>>) {
                container.push_back(dis(gen));
            } else {
                container.insert(dis(gen));
            }
        }
    });
    
    // Find performance (for associative containers)
    if constexpr (std::is_same_v<Container, std::set<int>> ||
                 std::is_same_v<Container, std::unordered_set<int>>) {
        std::cout << "  Find 1000 elements: ";
        MEASURE_TIME({
            for (int i = 0; i < 1000; ++i) {
                container.find(dis(gen));
            }
        });
    }
}

void run_comprehensive_benchmarks() {
    SECTION("Comprehensive Benchmarks");
    
    const size_t test_size = 50000;
    
    benchmark_container<std::vector<int>>("Vector", test_size);
    benchmark_container<std::deque<int>>("Deque", test_size);
    benchmark_container<std::list<int>>("List", test_size);
    benchmark_container<std::set<int>>("Set", test_size);
    benchmark_container<std::unordered_set<int>>("Unordered Set", test_size);
}

int main() {
    std::cout << "STL CONTAINERS PERFORMANCE ANALYSIS\n";
    std::cout << "===================================\n";
    
    test_sequence_containers();
    test_associative_containers();
    analyze_memory_usage();
    test_cache_performance();
    test_algorithm_performance();
    container_selection_guide();
    run_comprehensive_benchmarks();
    
    return 0;
}

/*
KEY PERFORMANCE INSIGHTS:
- Vector: Best for random access, cache-friendly, O(1) amortized push_back
- Deque: Good for front/back operations, slightly slower than vector
- List: Best for insertions/deletions, poor cache performance
- Set/Map: O(log n) operations, ordered iteration
- Unordered containers: O(1) average, can degrade to O(n) worst case
- Memory overhead: vector < deque < unordered < ordered < list
- Cache performance: vector > deque > unordered > ordered > list
- Choose container based on access patterns and performance requirements
*/
