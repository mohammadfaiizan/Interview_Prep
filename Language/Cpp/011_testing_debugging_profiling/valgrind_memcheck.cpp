/*
 * TESTING, DEBUGGING, PROFILING - VALGRIND MEMCHECK
 * 
 * This file demonstrates memory debugging using Valgrind's Memcheck tool
 * for detecting memory errors and leaks.
 * 
 * Compilation: g++ -std=c++17 -g -O0 valgrind_memcheck.cpp -o memcheck_demo
 * Usage: valgrind --tool=memcheck --leak-check=full ./memcheck_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>

// =============================================================================
// MEMORY ERROR DEMONSTRATIONS
// =============================================================================

void demonstrate_valgrind_basics() {
    std::cout << "\n=== Valgrind Memcheck Basics ===" << std::endl;
    
    std::cout << "\nBasic Valgrind commands:" << std::endl;
    std::cout << "valgrind ./program                    # Basic memory checking" << std::endl;
    std::cout << "valgrind --tool=memcheck ./program    # Explicit memcheck" << std::endl;
    std::cout << "valgrind --leak-check=full ./program  # Full leak detection" << std::endl;
    std::cout << "valgrind --show-leak-kinds=all ./program  # Show all leak types" << std::endl;
    
    std::cout << "\nMemcheck detects:" << std::endl;
    std::cout << "• Use of uninitialized memory" << std::endl;
    std::cout << "• Reading/writing memory after it has been freed" << std::endl;
    std::cout << "• Reading/writing off the end of malloc'd blocks" << std::endl;
    std::cout << "• Reading/writing inappropriate areas on the stack" << std::endl;
    std::cout << "• Memory leaks" << std::endl;
    std::cout << "• Mismatched use of malloc/new/new[] vs free/delete/delete[]" << std::endl;
}

void safe_memory_operations() {
    std::cout << "\n=== Safe Memory Operations ===" << std::endl;
    
    // Safe heap allocation
    {
        int* ptr = new int(42);
        std::cout << "Allocated and initialized: " << *ptr << std::endl;
        delete ptr;
        std::cout << "✓ Properly freed memory" << std::endl;
    }
    
    // Safe array allocation
    {
        int* arr = new int[10];
        for (int i = 0; i < 10; ++i) {
            arr[i] = i;
        }
        std::cout << "✓ Array properly initialized and accessed" << std::endl;
        delete[] arr;
    }
    
    // Safe smart pointer usage
    {
        auto smart_ptr = std::make_unique<int>(100);
        std::cout << "Smart pointer value: " << *smart_ptr << std::endl;
        // Automatic cleanup
        std::cout << "✓ Smart pointer automatically cleaned up" << std::endl;
    }
}

void demonstrate_memory_errors() {
    std::cout << "\n=== Memory Error Examples (Commented Out) ===" << std::endl;
    
    std::cout << "\n1. Use of uninitialized memory:" << std::endl;
    std::cout << "int x;" << std::endl;
    std::cout << "if (x == 42) { /* ... */ }  // x is uninitialized!" << std::endl;
    
    std::cout << "\n2. Use after free:" << std::endl;
    std::cout << "int* ptr = new int(42);" << std::endl;
    std::cout << "delete ptr;" << std::endl;
    std::cout << "*ptr = 24;  // Use after free!" << std::endl;
    
    std::cout << "\n3. Double free:" << std::endl;
    std::cout << "int* ptr = new int(42);" << std::endl;
    std::cout << "delete ptr;" << std::endl;
    std::cout << "delete ptr;  // Double free!" << std::endl;
    
    std::cout << "\n4. Buffer overflow:" << std::endl;
    std::cout << "int* arr = new int[10];" << std::endl;
    std::cout << "arr[15] = 42;  // Out of bounds!" << std::endl;
    std::cout << "delete[] arr;" << std::endl;
    
    std::cout << "\n5. Memory leak:" << std::endl;
    std::cout << "int* ptr = new int(42);" << std::endl;
    std::cout << "// Forgot to call delete ptr!" << std::endl;
    
    std::cout << "\n6. Mismatched allocation/deallocation:" << std::endl;
    std::cout << "int* ptr = new int[10];" << std::endl;
    std::cout << "delete ptr;  // Should be delete[]!" << std::endl;
}

// =============================================================================
// MEMORY LEAK DEMONSTRATIONS
// =============================================================================

class LeakyClass {
private:
    int* data_;
    size_t size_;
    
public:
    LeakyClass(size_t size) : size_(size) {
        data_ = new int[size];
        std::cout << "Allocated " << size << " integers" << std::endl;
    }
    
    // Missing destructor - this causes memory leak!
    // ~LeakyClass() { delete[] data_; }
    
    void set_value(size_t index, int value) {
        if (index < size_) {
            data_[index] = value;
        }
    }
    
    int get_value(size_t index) const {
        return (index < size_) ? data_[index] : 0;
    }
};

class ProperClass {
private:
    std::unique_ptr<int[]> data_;
    size_t size_;
    
public:
    ProperClass(size_t size) : data_(std::make_unique<int[]>(size)), size_(size) {
        std::cout << "Properly allocated " << size << " integers" << std::endl;
    }
    
    // Destructor automatically called for unique_ptr
    
    void set_value(size_t index, int value) {
        if (index < size_) {
            data_[index] = value;
        }
    }
    
    int get_value(size_t index) const {
        return (index < size_) ? data_[index] : 0;
    }
};

void demonstrate_memory_leaks() {
    std::cout << "\n=== Memory Leak Examples ===" << std::endl;
    
    std::cout << "\nProper memory management:" << std::endl;
    {
        ProperClass proper(100);
        proper.set_value(0, 42);
        std::cout << "Value: " << proper.get_value(0) << std::endl;
        // Automatic cleanup via RAII
    }
    std::cout << "✓ Memory properly cleaned up" << std::endl;
    
    std::cout << "\nLeaky class example (would leak in real execution):" << std::endl;
    std::cout << "LeakyClass leaky(100);  // This would leak memory!" << std::endl;
    std::cout << "// Missing destructor means memory is never freed" << std::endl;
}

// =============================================================================
// ADVANCED VALGRIND FEATURES
// =============================================================================

void demonstrate_advanced_valgrind() {
    std::cout << "\n=== Advanced Valgrind Features ===" << std::endl;
    
    std::cout << "\nSuppressions file example:" << std::endl;
    std::cout << R"(
# valgrind_suppressions.supp
{
   known_library_leak
   Memcheck:Leak
   match-leak-kinds: definite
   fun:malloc
   fun:library_function
   obj:/usr/lib/libknown.so
}

{
   false_positive_uninit
   Memcheck:Cond
   fun:strcmp
   fun:my_string_compare
}

# Usage: valgrind --suppressions=valgrind_suppressions.supp ./program
)" << std::endl;

    std::cout << "\nValgrind command line options:" << std::endl;
    std::cout << "--leak-check=full              # Detailed leak information" << std::endl;
    std::cout << "--show-leak-kinds=all          # Show all types of leaks" << std::endl;
    std::cout << "--track-origins=yes            # Track uninitialized values" << std::endl;
    std::cout << "--verbose                      # Verbose output" << std::endl;
    std::cout << "--log-file=valgrind.log        # Save output to file" << std::endl;
    std::cout << "--xml=yes --xml-file=out.xml   # XML output format" << std::endl;
    std::cout << "--gen-suppressions=all         # Generate suppression entries" << std::endl;
    
    std::cout << "\nLeak categories:" << std::endl;
    std::cout << "• Definitely lost: Memory not freed and no pointers to it" << std::endl;
    std::cout << "• Indirectly lost: Memory not freed but pointed to by lost memory" << std::endl;
    std::cout << "• Possibly lost: Memory not freed but pointers may exist" << std::endl;
    std::cout << "• Still reachable: Memory not freed but pointers still exist" << std::endl;
}

void demonstrate_valgrind_integration() {
    std::cout << "\n=== Build System Integration ===" << std::endl;
    
    std::cout << "\nCMake integration:" << std::endl;
    std::cout << R"(
# CMakeLists.txt
find_program(VALGRIND_EXECUTABLE valgrind)

if(VALGRIND_EXECUTABLE)
    # Custom target for memory checking
    add_custom_target(memcheck
        COMMAND ${VALGRIND_EXECUTABLE} 
                --tool=memcheck 
                --leak-check=full 
                --show-leak-kinds=all
                --track-origins=yes
                --verbose
                --log-file=memcheck.log
                $<TARGET_FILE:${PROJECT_NAME}>
        DEPENDS ${PROJECT_NAME}
        COMMENT "Running Valgrind memory check")
    
    # Integration with CTest
    add_test(NAME memcheck_test
        COMMAND ${VALGRIND_EXECUTABLE}
                --tool=memcheck
                --leak-check=full
                --error-exitcode=1
                $<TARGET_FILE:${PROJECT_NAME}>)
endif()
)" << std::endl;

    std::cout << "\nMakefile integration:" << std::endl;
    std::cout << R"(
# Makefile
VALGRIND_FLAGS = --tool=memcheck --leak-check=full --show-leak-kinds=all

memcheck: $(TARGET)
	valgrind $(VALGRIND_FLAGS) ./$(TARGET)

memcheck-log: $(TARGET)
	valgrind $(VALGRIND_FLAGS) --log-file=memcheck.log ./$(TARGET)

memcheck-xml: $(TARGET)
	valgrind $(VALGRIND_FLAGS) --xml=yes --xml-file=memcheck.xml ./$(TARGET)
)" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== Valgrind Best Practices ===" << std::endl;
    
    std::cout << "\n1. Compilation flags:" << std::endl;
    std::cout << "• Use -g for debug symbols" << std::endl;
    std::cout << "• Use -O0 to disable optimizations" << std::endl;
    std::cout << "• Avoid -fomit-frame-pointer" << std::endl;
    
    std::cout << "\n2. Testing strategy:" << std::endl;
    std::cout << "• Run Valgrind on debug builds" << std::endl;
    std::cout << "• Test with realistic data sets" << std::endl;
    std::cout << "• Include edge cases and error paths" << std::endl;
    std::cout << "• Run regularly during development" << std::endl;
    
    std::cout << "\n3. Performance considerations:" << std::endl;
    std::cout << "• Valgrind slows execution 10-50x" << std::endl;
    std::cout << "• Use smaller test data for Valgrind runs" << std::endl;
    std::cout << "• Consider timeout adjustments" << std::endl;
    
    std::cout << "\n4. CI/CD integration:" << std::endl;
    std::cout << "• Run Valgrind in separate CI job" << std::endl;
    std::cout << "• Use --error-exitcode=1 to fail builds" << std::endl;
    std::cout << "• Archive Valgrind logs as artifacts" << std::endl;
    std::cout << "• Set up notifications for new leaks" << std::endl;
}

void demonstrate_alternative_tools() {
    std::cout << "\n=== Alternative Memory Debugging Tools ===" << std::endl;
    
    std::cout << "\n1. AddressSanitizer (ASan):" << std::endl;
    std::cout << "• Faster than Valgrind (2-3x slowdown)" << std::endl;
    std::cout << "• Built into GCC/Clang" << std::endl;
    std::cout << "• Compile with -fsanitize=address" << std::endl;
    std::cout << "• Good for development and CI" << std::endl;
    
    std::cout << "\n2. Dr. Memory:" << std::endl;
    std::cout << "• Windows and Linux support" << std::endl;
    std::cout << "• Similar to Valgrind but faster" << std::endl;
    std::cout << "• Good alternative for cross-platform projects" << std::endl;
    
    std::cout << "\n3. Intel Inspector:" << std::endl;
    std::cout << "• Commercial tool with GUI" << std::endl;
    std::cout << "• Memory and threading error detection" << std::endl;
    std::cout << "• Good integration with Intel development tools" << std::endl;
    
    std::cout << "\n4. Static analysis tools:" << std::endl;
    std::cout << "• Clang Static Analyzer" << std::endl;
    std::cout << "• PVS-Studio" << std::endl;
    std::cout << "• Coverity" << std::endl;
    std::cout << "• Find issues without execution" << std::endl;
}

// =============================================================================
// PRACTICAL EXAMPLES
// =============================================================================

void demonstrate_common_scenarios() {
    std::cout << "\n=== Common Debugging Scenarios ===" << std::endl;
    
    std::cout << "\n1. String handling bugs:" << std::endl;
    {
        const char* source = "Hello, World!";
        char* dest = new char[10];  // Too small!
        
        // Safe version
        size_t len = std::strlen(source);
        delete[] dest;
        dest = new char[len + 1];
        std::strcpy(dest, source);
        
        std::cout << "Safely copied: " << dest << std::endl;
        delete[] dest;
    }
    
    std::cout << "\n2. Container memory management:" << std::endl;
    {
        std::vector<std::unique_ptr<int>> vec;
        for (int i = 0; i < 10; ++i) {
            vec.push_back(std::make_unique<int>(i));
        }
        std::cout << "✓ Smart pointers automatically cleaned up" << std::endl;
        // Automatic cleanup when vector goes out of scope
    }
    
    std::cout << "\n3. Exception safety:" << std::endl;
    {
        std::unique_ptr<int[]> data;
        try {
            data = std::make_unique<int[]>(1000);
            // Some operation that might throw
            std::cout << "✓ Exception-safe memory management" << std::endl;
        } catch (...) {
            // data automatically cleaned up even if exception occurs
            std::cout << "Exception handled, memory still safe" << std::endl;
        }
    }
}

int main() {
    std::cout << "VALGRIND MEMCHECK FOR MEMORY DEBUGGING\n";
    std::cout << "======================================\n";
    
    demonstrate_valgrind_basics();
    safe_memory_operations();
    demonstrate_memory_errors();
    demonstrate_memory_leaks();
    demonstrate_advanced_valgrind();
    demonstrate_valgrind_integration();
    demonstrate_best_practices();
    demonstrate_alternative_tools();
    demonstrate_common_scenarios();
    
    std::cout << "\nKey Valgrind Benefits:" << std::endl;
    std::cout << "• Detects memory errors that may not cause immediate crashes" << std::endl;
    std::cout << "• Provides detailed stack traces for error locations" << std::endl;
    std::cout << "• Helps ensure memory safety and prevent security vulnerabilities" << std::endl;
    std::cout << "• Works with existing binaries (no recompilation needed)" << std::endl;
    std::cout << "• Comprehensive leak detection and categorization" << std::endl;
    
    std::cout << "\nBest Practices Summary:" << std::endl;
    std::cout << "• Use RAII and smart pointers to prevent leaks" << std::endl;
    std::cout << "• Run Valgrind regularly during development" << std::endl;
    std::cout << "• Fix memory errors immediately when detected" << std::endl;
    std::cout << "• Integrate memory checking into CI/CD pipeline" << std::endl;
    std::cout << "• Consider alternative tools for different use cases" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Valgrind Memcheck is essential for memory safety verification
- Memory errors can be subtle and not cause immediate crashes
- RAII and smart pointers prevent most memory management issues
- Regular memory checking during development prevents bugs
- Integration with build systems enables automated checking
- Different tools have different strengths and use cases
- Memory safety is crucial for security and reliability
- Prevention is better than detection - use safe coding practices
*/
