/*
 * MODERN C++ - RANGES AND VIEWS (C++20)
 * 
 * Compilation: g++ -std=c++20 -Wall -Wextra ranges_and_views.cpp -o ranges_and_views
 */

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string>
#include <numeric>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

namespace rng = std::ranges;
namespace vw = std::views;

// =============================================================================
// 1. BASIC RANGES CONCEPTS
// =============================================================================

void demonstrate_basic_ranges() {
    SECTION("Basic Ranges Concepts");
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Traditional algorithm
    std::cout << "Traditional find: ";
    auto it = std::find(numbers.begin(), numbers.end(), 5);
    if (it != numbers.end()) {
        std::cout << *it << std::endl;
    }
    
    // Ranges algorithm
    std::cout << "Ranges find: ";
    auto it2 = rng::find(numbers, 5);
    if (it2 != numbers.end()) {
        std::cout << *it2 << std::endl;
    }
    
    // Range-based algorithms are more concise
    std::cout << "All even: " << std::boolalpha 
              << rng::all_of(numbers, [](int n) { return n % 2 == 0; }) << std::endl;
    
    std::cout << "Any greater than 8: " << std::boolalpha
              << rng::any_of(numbers, [](int n) { return n > 8; }) << std::endl;
    
    // Count with ranges
    auto even_count = rng::count_if(numbers, [](int n) { return n % 2 == 0; });
    std::cout << "Even numbers count: " << even_count << std::endl;
}

// =============================================================================
// 2. VIEWS AND LAZY EVALUATION
// =============================================================================

void demonstrate_views() {
    SECTION("Views and Lazy Evaluation");
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter view - lazy evaluation
    auto even_view = numbers | vw::filter([](int n) { return n % 2 == 0; });
    
    std::cout << "Even numbers (view): ";
    for (int n : even_view) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Transform view
    auto squared_view = numbers | vw::transform([](int n) { return n * n; });
    
    std::cout << "Squared numbers: ";
    for (int n : squared_view) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Chaining views
    auto filtered_squared = numbers 
        | vw::filter([](int n) { return n % 2 == 0; })
        | vw::transform([](int n) { return n * n; });
    
    std::cout << "Even numbers squared: ";
    for (int n : filtered_squared) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Take view
    auto first_five = numbers | vw::take(5);
    std::cout << "First 5: ";
    for (int n : first_five) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Drop view
    auto skip_three = numbers | vw::drop(3);
    std::cout << "Skip first 3: ";
    for (int n : skip_three) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 3. COMMON VIEWS
// =============================================================================

void demonstrate_common_views() {
    SECTION("Common Views");
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Reverse view
    std::cout << "Reversed: ";
    for (int n : numbers | vw::reverse) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Split view with strings
    std::string text = "hello,world,ranges,views";
    std::cout << "Split by comma: ";
    for (auto word : text | vw::split(',')) {
        for (char c : word) {
            std::cout << c;
        }
        std::cout << " ";
    }
    std::cout << std::endl;
    
    // Join view
    std::vector<std::string> words{"C++", "ranges", "are", "awesome"};
    std::cout << "Joined: ";
    for (char c : words | vw::join_with(' ')) {
        std::cout << c;
    }
    std::cout << std::endl;
    
    // Enumerate view (C++23, simulated)
    std::cout << "Enumerated: ";
    int index = 0;
    for (int n : numbers | vw::take(5)) {
        std::cout << "[" << index++ << "]=" << n << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 4. RANGE ADAPTORS AND COMPOSABILITY
// =============================================================================

void demonstrate_range_adaptors() {
    SECTION("Range Adaptors and Composability");
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    
    // Complex chain of operations
    auto complex_view = numbers
        | vw::filter([](int n) { return n > 3; })        // > 3
        | vw::transform([](int n) { return n * 2; })     // double
        | vw::filter([](int n) { return n % 4 == 0; })   // divisible by 4
        | vw::take(5);                                   // first 5
    
    std::cout << "Complex chain: ";
    for (int n : complex_view) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Keys and values (simulated for maps)
    std::vector<std::pair<int, std::string>> pairs{
        {1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}
    };
    
    std::cout << "Keys: ";
    for (int key : pairs | vw::transform([](const auto& p) { return p.first; })) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Values: ";
    for (const auto& value : pairs | vw::transform([](const auto& p) { return p.second; })) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // Chunking (grouping)
    std::cout << "Chunks of 3: ";
    // Note: chunk is C++23, simulating with take/drop pattern
    for (int start = 0; start < static_cast<int>(numbers.size()); start += 3) {
        std::cout << "[";
        for (int n : numbers | vw::drop(start) | vw::take(3)) {
            std::cout << n << " ";
        }
        std::cout << "] ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. RANGES WITH ALGORITHMS
// =============================================================================

void demonstrate_ranges_algorithms() {
    SECTION("Ranges with Algorithms");
    
    std::vector<int> numbers{5, 2, 8, 1, 9, 3};
    std::vector<int> sorted_copy;
    
    // Copy with ranges
    rng::copy(numbers, std::back_inserter(sorted_copy));
    
    // Sort with ranges
    rng::sort(sorted_copy);
    
    std::cout << "Original: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    
    std::cout << "Sorted copy: ";
    for (int n : sorted_copy) std::cout << n << " ";
    std::cout << std::endl;
    
    // Partial sort
    std::vector<int> partial_sorted = numbers;
    rng::partial_sort(partial_sorted, partial_sorted.begin() + 3);
    
    std::cout << "Partial sorted (first 3): ";
    for (int n : partial_sorted) std::cout << n << " ";
    std::cout << std::endl;
    
    // Binary search
    bool found = rng::binary_search(sorted_copy, 5);
    std::cout << "5 found in sorted: " << std::boolalpha << found << std::endl;
    
    // Min/max element
    auto [min_it, max_it] = rng::minmax_element(numbers);
    std::cout << "Min: " << *min_it << ", Max: " << *max_it << std::endl;
    
    // Accumulate (reduce)
    int sum = std::accumulate(numbers.begin(), numbers.end(), 0);
    std::cout << "Sum: " << sum << std::endl;
}

// =============================================================================
// 6. CUSTOM RANGES AND VIEWS
// =============================================================================

// Custom range generator
auto fibonacci_view(int count) {
    return vw::iota(0, count) | vw::transform([](int n) {
        if (n <= 1) return n;
        int a = 0, b = 1;
        for (int i = 2; i <= n; ++i) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    });
}

void demonstrate_custom_ranges() {
    SECTION("Custom Ranges and Views");
    
    // Infinite range (well, large range)
    std::cout << "First 10 natural numbers: ";
    for (int n : vw::iota(1) | vw::take(10)) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Fibonacci sequence
    std::cout << "First 10 Fibonacci numbers: ";
    for (int n : fibonacci_view(10)) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    // Repeat view
    std::cout << "Repeat 'A' 5 times: ";
    for (char c : vw::repeat('A') | vw::take(5)) {
        std::cout << c << " ";
    }
    std::cout << std::endl;
    
    // Empty view
    std::cout << "Empty range size: ";
    auto empty_range = vw::empty<int>;
    std::cout << rng::distance(empty_range) << std::endl;
    
    // Single view
    std::cout << "Single element: ";
    for (int n : vw::single(42)) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 7. PRACTICAL EXAMPLES
// =============================================================================

void demonstrate_practical_examples() {
    SECTION("Practical Examples");
    
    // Data processing pipeline
    std::vector<std::string> words{
        "hello", "world", "C++", "ranges", "are", "powerful", "and", "elegant"
    };
    
    // Find long words, convert to uppercase, sort
    std::vector<std::string> processed;
    
    auto long_words = words 
        | vw::filter([](const std::string& s) { return s.length() > 3; })
        | vw::transform([](std::string s) { 
            rng::transform(s, s.begin(), ::toupper);
            return s;
        });
    
    rng::copy(long_words, std::back_inserter(processed));
    rng::sort(processed);
    
    std::cout << "Long words (uppercase, sorted): ";
    for (const auto& word : processed) {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    
    // Statistical analysis
    std::vector<double> data{1.5, 2.3, 3.7, 2.1, 4.2, 1.8, 3.9, 2.7, 4.1, 3.3};
    
    auto stats_view = data | vw::filter([](double x) { return x > 2.0; });
    
    double sum = 0.0;
    int count = 0;
    for (double x : stats_view) {
        sum += x;
        count++;
    }
    
    std::cout << "Average of values > 2.0: " << (count > 0 ? sum / count : 0.0) << std::endl;
    
    // Text processing
    std::string text = "The quick brown fox jumps over the lazy dog";
    
    auto word_lengths = text 
        | vw::split(' ')
        | vw::transform([](auto word_range) {
            return static_cast<int>(rng::distance(word_range));
        });
    
    std::cout << "Word lengths: ";
    for (int len : word_lengths) {
        std::cout << len << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "RANGES AND VIEWS (C++20) TUTORIAL\n";
    std::cout << "=================================\n";
    
    demonstrate_basic_ranges();
    demonstrate_views();
    demonstrate_common_views();
    demonstrate_range_adaptors();
    demonstrate_ranges_algorithms();
    demonstrate_custom_ranges();
    demonstrate_practical_examples();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Ranges provide a more functional approach to algorithms
- Views are lazy-evaluated range adaptors
- Composable with | operator for readable pipelines
- std::ranges algorithms work directly on ranges
- Views don't own data - they provide different perspectives
- Major categories: filter, transform, take, drop, reverse, split, join
- Infinite ranges possible with lazy evaluation
- C++20 feature requiring modern compiler support
- Significantly improves code readability and expressiveness
*/
