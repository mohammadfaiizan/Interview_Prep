/*
 * STL DEEP DIVE - ASSOCIATIVE VS UNORDERED CONTAINERS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra associative_unordered.cpp -o associative_unordered
 */

#include <iostream>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <random>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC COMPARISON
// =============================================================================

void demonstrate_basic_comparison() {
    SECTION("Basic Comparison");
    
    std::cout << "ASSOCIATIVE CONTAINERS (set, map):" << std::endl;
    std::cout << "- Tree-based (typically Red-Black tree)" << std::endl;
    std::cout << "- Ordered iteration" << std::endl;
    std::cout << "- O(log n) operations" << std::endl;
    std::cout << "- Stable performance" << std::endl;
    
    std::cout << "\nUNORDERED CONTAINERS (unordered_set, unordered_map):" << std::endl;
    std::cout << "- Hash table based" << std::endl;
    std::cout << "- Unordered iteration" << std::endl;
    std::cout << "- O(1) average operations" << std::endl;
    std::cout << "- Performance depends on hash quality" << std::endl;
    
    // Demonstrate ordering difference
    std::set<int> ordered_set = {5, 2, 8, 1, 9, 3};
    std::unordered_set<int> unordered_set = {5, 2, 8, 1, 9, 3};
    
    std::cout << "\nOrdered set iteration: ";
    for (int x : ordered_set) std::cout << x << " ";
    std::cout << std::endl;
    
    std::cout << "Unordered set iteration: ";
    for (int x : unordered_set) std::cout << x << " ";
    std::cout << std::endl;
}

// =============================================================================
// 2. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison");
    
    const size_t size = 100000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    std::vector<int> test_data;
    for (size_t i = 0; i < size; ++i) {
        test_data.push_back(dis(gen));
    }
    
    // Set vs unordered_set insertion
    std::set<int> s;
    auto start = std::chrono::high_resolution_clock::now();
    for (int value : test_data) {
        s.insert(value);
    }
    auto set_insert_time = std::chrono::high_resolution_clock::now() - start;
    
    std::unordered_set<int> us;
    start = std::chrono::high_resolution_clock::now();
    for (int value : test_data) {
        us.insert(value);
    }
    auto uset_insert_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Insertion of " << size << " elements:" << std::endl;
    std::cout << "Set: " << std::chrono::duration_cast<std::chrono::milliseconds>(set_insert_time).count() << " ms" << std::endl;
    std::cout << "Unordered set: " << std::chrono::duration_cast<std::chrono::milliseconds>(uset_insert_time).count() << " ms" << std::endl;
    
    // Find performance
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        s.find(dis(gen));
    }
    auto set_find_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        us.find(dis(gen));
    }
    auto uset_find_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "\nFind 10000 elements:" << std::endl;
    std::cout << "Set: " << std::chrono::duration_cast<std::chrono::microseconds>(set_find_time).count() << " μs" << std::endl;
    std::cout << "Unordered set: " << std::chrono::duration_cast<std::chrono::microseconds>(uset_find_time).count() << " μs" << std::endl;
}

// =============================================================================
// 3. HASH TABLE INTERNALS
// =============================================================================

void demonstrate_hash_internals() {
    SECTION("Hash Table Internals");
    
    std::unordered_set<int> us;
    
    std::cout << "Hash table properties:" << std::endl;
    std::cout << "Initial bucket count: " << us.bucket_count() << std::endl;
    std::cout << "Initial load factor: " << us.load_factor() << std::endl;
    std::cout << "Max load factor: " << us.max_load_factor() << std::endl;
    
    // Add elements and watch rehashing
    for (int i = 1; i <= 20; ++i) {
        us.insert(i);
        if (i % 5 == 0) {
            std::cout << "After " << i << " insertions:" << std::endl;
            std::cout << "  Bucket count: " << us.bucket_count() << std::endl;
            std::cout << "  Load factor: " << us.load_factor() << std::endl;
        }
    }
    
    // Show bucket distribution
    std::cout << "\nBucket distribution:" << std::endl;
    for (size_t i = 0; i < us.bucket_count(); ++i) {
        std::cout << "Bucket " << i << ": " << us.bucket_size(i) << " elements" << std::endl;
    }
}

// =============================================================================
// 4. CUSTOM HASH AND EQUALITY
// =============================================================================

struct Point {
    int x, y;
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

void demonstrate_custom_hash() {
    SECTION("Custom Hash and Equality");
    
    std::unordered_set<Point, PointHash> point_set;
    
    point_set.insert({1, 2});
    point_set.insert({3, 4});
    point_set.insert({1, 2}); // Duplicate, won't be inserted
    
    std::cout << "Point set size: " << point_set.size() << std::endl;
    
    // Custom hash for map
    std::unordered_map<Point, std::string, PointHash> point_map;
    point_map[{1, 2}] = "Point A";
    point_map[{3, 4}] = "Point B";
    
    std::cout << "Point map contents:" << std::endl;
    for (const auto& [point, name] : point_map) {
        std::cout << "(" << point.x << ", " << point.y << ") -> " << name << std::endl;
    }
}

// =============================================================================
// 5. WHEN TO USE WHICH
// =============================================================================

void usage_guidelines() {
    SECTION("Usage Guidelines");
    
    std::cout << "USE ASSOCIATIVE CONTAINERS (set, map) WHEN:" << std::endl;
    std::cout << "✓ Need ordered iteration" << std::endl;
    std::cout << "✓ Need range queries (lower_bound, upper_bound)" << std::endl;
    std::cout << "✓ Want predictable O(log n) performance" << std::endl;
    std::cout << "✓ Memory usage is critical" << std::endl;
    std::cout << "✓ Need set operations (union, intersection)" << std::endl;
    
    std::cout << "\nUSE UNORDERED CONTAINERS (unordered_set, unordered_map) WHEN:" << std::endl;
    std::cout << "✓ Need fastest average lookup/insertion" << std::endl;
    std::cout << "✓ Don't need ordered iteration" << std::endl;
    std::cout << "✓ Have good hash function for your type" << std::endl;
    std::cout << "✓ Can tolerate worst-case O(n) performance" << std::endl;
    std::cout << "✓ Working with simple key types (int, string)" << std::endl;
    
    // Demonstrate range operations (only available for associative)
    std::set<int> s = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    
    auto lower = s.lower_bound(7);
    auto upper = s.upper_bound(13);
    
    std::cout << "\nRange query example (set only):" << std::endl;
    std::cout << "Elements in range [7, 13]: ";
    for (auto it = lower; it != upper; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 6. MULTISET AND MULTIMAP
// =============================================================================

void demonstrate_multi_containers() {
    SECTION("Multi-containers");
    
    std::multiset<int> ms = {1, 2, 2, 3, 3, 3, 4};
    std::unordered_multiset<int> ums = {1, 2, 2, 3, 3, 3, 4};
    
    std::cout << "Multiset (allows duplicates):" << std::endl;
    std::cout << "Contents: ";
    for (int x : ms) std::cout << x << " ";
    std::cout << std::endl;
    
    std::cout << "Count of 3: " << ms.count(3) << std::endl;
    
    // Equal range
    auto range = ms.equal_range(3);
    std::cout << "All occurrences of 3: ";
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Multimap example
    std::multimap<std::string, int> grades;
    grades.insert({"Alice", 85});
    grades.insert({"Bob", 92});
    grades.insert({"Alice", 78}); // Alice has multiple grades
    
    std::cout << "\nMultimap example (student grades):" << std::endl;
    for (const auto& [name, grade] : grades) {
        std::cout << name << ": " << grade << std::endl;
    }
}

int main() {
    std::cout << "ASSOCIATIVE VS UNORDERED CONTAINERS\n";
    std::cout << "===================================\n";
    
    demonstrate_basic_comparison();
    performance_comparison();
    demonstrate_hash_internals();
    demonstrate_custom_hash();
    usage_guidelines();
    demonstrate_multi_containers();
    
    return 0;
}

/*
KEY TAKEAWAYS:
- Associative: ordered, O(log n), stable performance, range queries
- Unordered: fast average O(1), hash-based, no ordering
- Choose based on performance needs and ordering requirements
- Custom types need hash function and equality for unordered containers
- Multi-containers allow duplicate keys
- Hash table performance depends on hash quality and load factor
- Memory overhead: unordered > associative for small datasets
- Worst-case performance: associative better guarantees
*/
