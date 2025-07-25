/*
 * MEMORY AND POINTERS - POINTERS VS REFERENCES
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra pointers_vs_references.cpp -o pointers_vs_references
 */

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC DIFFERENCES
// =============================================================================

void demonstrate_basic_differences() {
    SECTION("Basic Differences");
    
    int value = 42;
    
    // Pointer syntax
    int* ptr = &value;
    std::cout << "Pointer value: " << *ptr << std::endl;
    std::cout << "Pointer address: " << ptr << std::endl;
    
    // Reference syntax
    int& ref = value;
    std::cout << "Reference value: " << ref << std::endl;
    std::cout << "Reference address: " << &ref << std::endl;
    
    // Key differences
    int* ptr2 = nullptr;        // Pointers can be null
    // int& ref2;               // Error: references must be initialized
    
    int other = 100;
    ptr = &other;               // Pointer can be reassigned
    ref = other;                // This assigns value, doesn't rebind reference
    
    std::cout << "Original value after assignment: " << value << std::endl;
}

// =============================================================================
// 2. FUNCTION PARAMETERS
// =============================================================================

void modify_by_pointer(int* x) {
    if (x) *x = 999;
}

void modify_by_reference(int& x) {
    x = 999;
}

void demonstrate_function_parameters() {
    SECTION("Function Parameters");
    
    int val1 = 42, val2 = 42;
    
    modify_by_pointer(&val1);
    modify_by_reference(val2);
    
    std::cout << "Modified by pointer: " << val1 << std::endl;
    std::cout << "Modified by reference: " << val2 << std::endl;
    
    // Null safety
    modify_by_pointer(nullptr);  // Safe: function checks for null
    // modify_by_reference(???);  // References cannot be null
}

// =============================================================================
// 3. CONST CORRECTNESS
// =============================================================================

void demonstrate_const_correctness() {
    SECTION("Const Correctness");
    
    int value = 42;
    const int const_value = 100;
    
    // Pointer variations
    const int* ptr_to_const = &value;        // Cannot modify value
    int* const const_ptr = &value;           // Cannot reassign pointer
    const int* const const_ptr_to_const = &const_value;  // Both const
    
    // Reference variations
    const int& const_ref = value;            // Cannot modify value
    const int& temp_ref = 123;               // Can bind to temporary
    
    std::cout << "Const reference to temporary: " << temp_ref << std::endl;
}

// =============================================================================
// 4. ARRAYS AND CONTAINERS
// =============================================================================

void demonstrate_arrays() {
    SECTION("Arrays and Containers");
    
    int arr[5] = {1, 2, 3, 4, 5};
    
    // Array decay to pointer
    int* ptr = arr;
    std::cout << "Array via pointer: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << ptr[i] << " ";
    }
    std::cout << std::endl;
    
    // Reference to array
    int (&arr_ref)[5] = arr;
    std::cout << "Array size via reference: " << sizeof(arr_ref) << std::endl;
    std::cout << "Pointer size: " << sizeof(ptr) << std::endl;
    
    // Container with references
    std::vector<int> vec{1, 2, 3, 4, 5};
    for (const int& element : vec) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. PERFORMANCE AND BEST PRACTICES
// =============================================================================

class LargeObject {
    char data[1000];
public:
    void process() const {}
};

void process_by_value(LargeObject obj) { obj.process(); }
void process_by_pointer(const LargeObject* obj) { if (obj) obj->process(); }
void process_by_reference(const LargeObject& obj) { obj.process(); }

void demonstrate_performance() {
    SECTION("Performance and Best Practices");
    
    LargeObject large_obj;
    
    std::cout << "Processing methods:" << std::endl;
    std::cout << "1. By value - expensive copy" << std::endl;
    std::cout << "2. By pointer - cheap, needs null check" << std::endl;
    std::cout << "3. By reference - cheap, no null check" << std::endl;
    
    process_by_reference(large_obj);  // Preferred for large objects
    process_by_pointer(&large_obj);   // When null is possible
    
    std::cout << "\nBest Practices:" << std::endl;
    std::cout << "- Prefer references over pointers when possible" << std::endl;
    std::cout << "- Use pointers when null values are meaningful" << std::endl;
    std::cout << "- Use const correctness" << std::endl;
    std::cout << "- Pass large objects by reference" << std::endl;
}

// =============================================================================
// 6. PRACTICAL EXAMPLES
// =============================================================================

struct ListNode {
    int data;
    ListNode* next;  // Pointer for optional next node
    
    ListNode(int val) : data(val), next(nullptr) {}
};

void demonstrate_practical_examples() {
    SECTION("Practical Examples");
    
    // Linked list with pointers
    ListNode node1(1);
    ListNode node2(2);
    ListNode node3(3);
    
    node1.next = &node2;
    node2.next = &node3;
    
    std::cout << "Linked list: ";
    ListNode* current = &node1;
    while (current) {
        std::cout << current->data << " ";
        current = current->next;
    }
    std::cout << std::endl;
    
    // Function pointers
    auto add = [](int a, int b) { return a + b; };
    int (*operation)(int, int) = add;
    std::cout << "Function pointer result: " << operation(5, 3) << std::endl;
}

int main() {
    std::cout << "POINTERS VS REFERENCES TUTORIAL\n";
    std::cout << "===============================\n";
    
    demonstrate_basic_differences();
    demonstrate_function_parameters();
    demonstrate_const_correctness();
    demonstrate_arrays();
    demonstrate_performance();
    demonstrate_practical_examples();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Pointers store addresses, references are aliases
- Pointers can be null, references cannot
- Pointers can be reassigned, references cannot
- References must be initialized
- Prefer references for function parameters when possible
- Use pointers when null values are meaningful
- Both enable efficient parameter passing
- Const correctness applies to both
*/
