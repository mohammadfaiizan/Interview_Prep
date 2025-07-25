/*
 * TESTING, DEBUGGING, PROFILING - SANITIZERS (ASAN, TSAN, UBSAN)
 * 
 * This file demonstrates the use of various sanitizers for detecting
 * memory errors, thread safety issues, and undefined behavior.
 * 
 * Compilation examples:
 * AddressSanitizer: g++ -std=c++17 -g -fsanitize=address sanitizers.cpp -o sanitizers_asan
 * ThreadSanitizer:  g++ -std=c++17 -g -fsanitize=thread sanitizers.cpp -o sanitizers_tsan
 * UBSanitizer:      g++ -std=c++17 -g -fsanitize=undefined sanitizers.cpp -o sanitizers_ubsan
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>

// =============================================================================
// DEBUGGING MACROS AND UTILITIES
// =============================================================================

#define SANITIZER_DEMO(name) \
    std::cout << "\n=== " << name << " ===" << std::endl

#define SAFE_DEMO(description, code) \
    std::cout << "\n" << description << ":" << std::endl; \
    try { \
        code; \
        std::cout << "✓ No issues detected" << std::endl; \
    } catch (const std::exception& e) { \
        std::cout << "✗ Exception: " << e.what() << std::endl; \
    }

#define UNSAFE_DEMO(description, code) \
    std::cout << "\n" << description << " (WILL TRIGGER SANITIZER):" << std::endl; \
    std::cout << "Code: " << #code << std::endl; \
    /* Commented out to prevent crashes during normal execution */ \
    /* code; */

// =============================================================================
// ADDRESS SANITIZER DEMONSTRATIONS
// =============================================================================

void demonstrate_address_sanitizer() {
    SANITIZER_DEMO("AddressSanitizer (ASan) Examples");
    
    std::cout << "\nAddressSanitizer detects:" << std::endl;
    std::cout << "• Heap buffer overflow/underflow" << std::endl;
    std::cout << "• Stack buffer overflow" << std::endl;
    std::cout << "• Use-after-free" << std::endl;
    std::cout << "• Double-free" << std::endl;
    std::cout << "• Memory leaks" << std::endl;
    
    // 1. Heap buffer overflow
    SAFE_DEMO("Safe heap access", {
        std::vector<int> vec(10);
        for (int i = 0; i < 10; ++i) {
            vec[i] = i;
        }
        std::cout << "Safely accessed all elements" << std::endl;
    });
    
    UNSAFE_DEMO("Heap buffer overflow", {
        int* arr = new int[10];
        arr[15] = 42;  // Out of bounds access
        delete[] arr;
    });
    
    // 2. Stack buffer overflow
    UNSAFE_DEMO("Stack buffer overflow", {
        int arr[10];
        arr[15] = 42;  // Out of bounds access
    });
    
    // 3. Use-after-free
    UNSAFE_DEMO("Use-after-free", {
        int* ptr = new int(42);
        delete ptr;
        *ptr = 24;  // Use after free
    });
    
    // 4. Double-free
    UNSAFE_DEMO("Double-free", {
        int* ptr = new int(42);
        delete ptr;
        delete ptr;  // Double free
    });
    
    // 5. Memory leak detection (requires ASAN_OPTIONS=detect_leaks=1)
    UNSAFE_DEMO("Memory leak", {
        int* ptr = new int(42);
        // Intentionally not calling delete ptr;
    });
    
    std::cout << "\nASan Compilation and Runtime Options:" << std::endl;
    std::cout << "Compilation: g++ -fsanitize=address -g -O1 program.cpp" << std::endl;
    std::cout << "Runtime options (ASAN_OPTIONS environment variable):" << std::endl;
    std::cout << "  detect_leaks=1              # Enable leak detection" << std::endl;
    std::cout << "  abort_on_error=1            # Abort on first error" << std::endl;
    std::cout << "  halt_on_error=1             # Halt on error" << std::endl;
    std::cout << "  check_initialization_order=1 # Check global init order" << std::endl;
    std::cout << "  strict_init_order=1         # Strict initialization order" << std::endl;
    std::cout << "  detect_stack_use_after_return=1 # Detect stack UAF" << std::endl;
}

// =============================================================================
// THREAD SANITIZER DEMONSTRATIONS
// =============================================================================

class ThreadUnsafeCounter {
private:
    int count_ = 0;  // Not thread-safe!
    
public:
    void increment() {
        count_++;  // Race condition!
    }
    
    void decrement() {
        count_--;  // Race condition!
    }
    
    int get_count() const {
        return count_;  // Race condition!
    }
};

class ThreadSafeCounter {
private:
    mutable std::mutex mutex_;
    int count_ = 0;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        count_++;
    }
    
    void decrement() {
        std::lock_guard<std::mutex> lock(mutex_);
        count_--;
    }
    
    int get_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
};

class AtomicCounter {
private:
    std::atomic<int> count_{0};
    
public:
    void increment() {
        count_.fetch_add(1);
    }
    
    void decrement() {
        count_.fetch_sub(1);
    }
    
    int get_count() const {
        return count_.load();
    }
};

void worker_thread_unsafe(ThreadUnsafeCounter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
        if (i % 2 == 0) {
            counter.decrement();
        }
    }
}

void worker_thread_safe(ThreadSafeCounter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
        if (i % 2 == 0) {
            counter.decrement();
        }
    }
}

void worker_thread_atomic(AtomicCounter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
        if (i % 2 == 0) {
            counter.decrement();
        }
    }
}

void demonstrate_thread_sanitizer() {
    SANITIZER_DEMO("ThreadSanitizer (TSan) Examples");
    
    std::cout << "\nThreadSanitizer detects:" << std::endl;
    std::cout << "• Data races" << std::endl;
    std::cout << "• Deadlocks" << std::endl;
    std::cout << "• Thread leaks" << std::endl;
    std::cout << "• Signal-unsafe function calls" << std::endl;
    
    const int num_threads = 4;
    const int iterations = 1000;
    
    // 1. Thread-safe implementation
    SAFE_DEMO("Thread-safe counter with mutex", {
        ThreadSafeCounter safe_counter;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_thread_safe, std::ref(safe_counter), iterations);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Final count: " << safe_counter.get_count() << std::endl;
    });
    
    // 2. Atomic implementation
    SAFE_DEMO("Thread-safe counter with atomics", {
        AtomicCounter atomic_counter;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_thread_atomic, std::ref(atomic_counter), iterations);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Final count: " << atomic_counter.get_count() << std::endl;
    });
    
    // 3. Thread-unsafe implementation (would trigger TSan)
    std::cout << "\nThread-unsafe counter (WOULD TRIGGER TSAN):" << std::endl;
    std::cout << "This would cause data races if executed with multiple threads" << std::endl;
    std::cout << "Code example:" << std::endl;
    std::cout << R"(
ThreadUnsafeCounter unsafe_counter;
std::vector<std::thread> threads;

for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(worker_thread_unsafe, std::ref(unsafe_counter), iterations);
}

for (auto& t : threads) {
    t.join();
}
)" << std::endl;
    
    std::cout << "\nTSan Compilation and Runtime Options:" << std::endl;
    std::cout << "Compilation: g++ -fsanitize=thread -g -O1 program.cpp" << std::endl;
    std::cout << "Runtime options (TSAN_OPTIONS environment variable):" << std::endl;
    std::cout << "  halt_on_error=1             # Stop on first error" << std::endl;
    std::cout << "  abort_on_error=1            # Abort on error" << std::endl;
    std::cout << "  report_bugs=1               # Report bugs" << std::endl;
    std::cout << "  history_size=7              # Race detection window" << std::endl;
    std::cout << "  detect_deadlocks=1          # Enable deadlock detection" << std::endl;
}

// =============================================================================
// UNDEFINED BEHAVIOR SANITIZER DEMONSTRATIONS
// =============================================================================

void demonstrate_undefined_behavior_sanitizer() {
    SANITIZER_DEMO("UndefinedBehaviorSanitizer (UBSan) Examples");
    
    std::cout << "\nUBSan detects:" << std::endl;
    std::cout << "• Integer overflow/underflow" << std::endl;
    std::cout << "• Division by zero" << std::endl;
    std::cout << "• Invalid shifts" << std::endl;
    std::cout << "• Null pointer dereference" << std::endl;
    std::cout << "• Invalid enum values" << std::endl;
    std::cout << "• Array bounds violations" << std::endl;
    
    // 1. Safe arithmetic operations
    SAFE_DEMO("Safe arithmetic", {
        int a = 100;
        int b = 200;
        int sum = a + b;
        std::cout << "Sum: " << sum << std::endl;
        
        if (b != 0) {
            int division = a / b;
            std::cout << "Division: " << division << std::endl;
        }
    });
    
    // 2. Integer overflow (would trigger UBSan)
    UNSAFE_DEMO("Integer overflow", {
        int max_int = std::numeric_limits<int>::max();
        int overflow = max_int + 1;  // Undefined behavior
        std::cout << "Overflow result: " << overflow << std::endl;
    });
    
    // 3. Division by zero
    UNSAFE_DEMO("Division by zero", {
        int a = 42;
        int b = 0;
        int result = a / b;  // Undefined behavior
        std::cout << "Result: " << result << std::endl;
    });
    
    // 4. Invalid shift operations
    UNSAFE_DEMO("Invalid shift", {
        int value = 1;
        int shift = 32;  // Shifting by >= width is UB for int
        int result = value << shift;
        std::cout << "Shift result: " << result << std::endl;
    });
    
    // 5. Null pointer dereference
    UNSAFE_DEMO("Null pointer dereference", {
        int* ptr = nullptr;
        int value = *ptr;  // Undefined behavior
        std::cout << "Value: " << value << std::endl;
    });
    
    // 6. Array bounds (in some configurations)
    UNSAFE_DEMO("Array bounds violation", {
        int arr[5] = {1, 2, 3, 4, 5};
        int value = arr[10];  // Out of bounds
        std::cout << "Value: " << value << std::endl;
    });
    
    std::cout << "\nUBSan Compilation Options:" << std::endl;
    std::cout << "Basic: g++ -fsanitize=undefined program.cpp" << std::endl;
    std::cout << "Specific checks:" << std::endl;
    std::cout << "  -fsanitize=signed-integer-overflow" << std::endl;
    std::cout << "  -fsanitize=unsigned-integer-overflow" << std::endl;
    std::cout << "  -fsanitize=shift" << std::endl;
    std::cout << "  -fsanitize=integer-divide-by-zero" << std::endl;
    std::cout << "  -fsanitize=null" << std::endl;
    std::cout << "  -fsanitize=bounds" << std::endl;
    std::cout << "  -fsanitize=alignment" << std::endl;
    std::cout << "  -fsanitize=object-size" << std::endl;
    std::cout << "  -fsanitize=vla-bound" << std::endl;
}

// =============================================================================
// MEMORY SANITIZER DEMONSTRATION
// =============================================================================

void demonstrate_memory_sanitizer() {
    SANITIZER_DEMO("MemorySanitizer (MSan) Examples");
    
    std::cout << "\nMemorySanitizer detects:" << std::endl;
    std::cout << "• Use of uninitialized memory" << std::endl;
    std::cout << "• Reading from uninitialized variables" << std::endl;
    std::cout << "• Passing uninitialized values to functions" << std::endl;
    
    // Safe: Initialized memory
    SAFE_DEMO("Initialized memory usage", {
        int initialized_value = 42;
        std::cout << "Initialized value: " << initialized_value << std::endl;
        
        std::vector<int> vec(10, 0);  // Initialized to 0
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i] = i;
        }
        std::cout << "Vector initialized and used safely" << std::endl;
    });
    
    // Unsafe: Uninitialized memory (would trigger MSan)
    std::cout << "\nUninitialized memory usage (WOULD TRIGGER MSAN):" << std::endl;
    std::cout << R"(
int uninitialized_value;  // Not initialized!
if (uninitialized_value > 0) {  // Use of uninitialized memory
    std::cout << "Value is positive" << std::endl;
}

int* heap_memory = new int[10];  // Uninitialized heap memory
int sum = 0;
for (int i = 0; i < 10; ++i) {
    sum += heap_memory[i];  // Use of uninitialized memory
}
delete[] heap_memory;
)" << std::endl;
    
    std::cout << "\nMSan Compilation:" << std::endl;
    std::cout << "g++ -fsanitize=memory -g -O1 program.cpp" << std::endl;
    std::cout << "Note: MSan requires all libraries to be MSan-instrumented" << std::endl;
    std::cout << "Runtime options (MSAN_OPTIONS):" << std::endl;
    std::cout << "  halt_on_error=1" << std::endl;
    std::cout << "  abort_on_error=1" << std::endl;
    std::cout << "  print_stats=1" << std::endl;
}

// =============================================================================
// COMBINING SANITIZERS AND BEST PRACTICES
// =============================================================================

void demonstrate_sanitizer_best_practices() {
    SANITIZER_DEMO("Sanitizer Best Practices");
    
    std::cout << "\n1. Sanitizer Compatibility:" << std::endl;
    std::cout << "• ASan + UBSan: Compatible" << std::endl;
    std::cout << "• ASan + TSan: NOT compatible" << std::endl;
    std::cout << "• TSan + UBSan: Compatible" << std::endl;
    std::cout << "• MSan + others: Generally not compatible" << std::endl;
    
    std::cout << "\n2. CI/CD Integration:" << std::endl;
    std::cout << R"(
# CMakeLists.txt example
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()

if(ENABLE_TSAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()

if(ENABLE_UBSAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined)
endif()
)" << std::endl;

    std::cout << "\n3. Testing Strategy:" << std::endl;
    std::cout << "• Run different sanitizers in separate CI jobs" << std::endl;
    std::cout << "• Use ASan for general memory safety testing" << std::endl;
    std::cout << "• Use TSan for multithreaded code testing" << std::endl;
    std::cout << "• Use UBSan for catching undefined behavior" << std::endl;
    std::cout << "• Use MSan for uninitialized memory detection" << std::endl;
    
    std::cout << "\n4. Performance Considerations:" << std::endl;
    std::cout << "• ASan: ~2x slowdown, ~3x memory usage" << std::endl;
    std::cout << "• TSan: ~5-15x slowdown, ~5-10x memory usage" << std::endl;
    std::cout << "• UBSan: ~20% slowdown (minimal overhead)" << std::endl;
    std::cout << "• MSan: ~3x slowdown, ~3x memory usage" << std::endl;
    
    std::cout << "\n5. Suppression Files:" << std::endl;
    std::cout << "Create suppression files for known false positives:" << std::endl;
    std::cout << R"(
# asan_suppressions.txt
leak:known_library_leak
leak:third_party_library

# tsan_suppressions.txt
race:ThirdPartyLibrary::*
deadlock:KnownDeadlockPattern

# Usage:
# ASAN_OPTIONS=suppressions=asan_suppressions.txt ./program
# TSAN_OPTIONS=suppressions=tsan_suppressions.txt ./program
)" << std::endl;
}

void demonstrate_real_world_examples() {
    SANITIZER_DEMO("Real-World Sanitizer Usage");
    
    std::cout << "\n1. Example Bug Caught by ASan:" << std::endl;
    std::cout << R"(
// Bug: Buffer overflow in string processing
void process_string(const char* input) {
    char buffer[256];
    strcpy(buffer, input);  // No bounds checking!
    // ... process buffer
}

// ASan would report:
// ERROR: AddressSanitizer: heap-buffer-overflow
// WRITE of size 1 at 0x... thread T0
)" << std::endl;

    std::cout << "\n2. Example Bug Caught by TSan:" << std::endl;
    std::cout << R"(
// Bug: Race condition in singleton
class Singleton {
    static Singleton* instance;
public:
    static Singleton* getInstance() {
        if (!instance) {  // Race condition here!
            instance = new Singleton();
        }
        return instance;
    }
};

// TSan would report:
// WARNING: ThreadSanitizer: data race
// Read of size 8 at 0x... by thread T1
// Previous write of size 8 at 0x... by thread T2
)" << std::endl;

    std::cout << "\n3. Example Bug Caught by UBSan:" << std::endl;
    std::cout << R"(
// Bug: Integer overflow in calculation
int calculate_area(int width, int height) {
    return width * height;  // May overflow!
}

int main() {
    int area = calculate_area(100000, 100000);  // Overflow!
    return 0;
}

// UBSan would report:
// runtime error: signed integer overflow: 
// 100000 * 100000 cannot be represented in type 'int'
)" << std::endl;
}

int main() {
    std::cout << "SANITIZERS FOR C++ DEBUGGING\n";
    std::cout << "=============================\n";
    
    demonstrate_address_sanitizer();
    demonstrate_thread_sanitizer();
    demonstrate_undefined_behavior_sanitizer();
    demonstrate_memory_sanitizer();
    demonstrate_sanitizer_best_practices();
    demonstrate_real_world_examples();
    
    std::cout << "\nKey Sanitizer Benefits:" << std::endl;
    std::cout << "• Catch bugs early in development cycle" << std::endl;
    std::cout << "• Provide detailed error reports with stack traces" << std::endl;
    std::cout << "• Integrate well with CI/CD pipelines" << std::endl;
    std::cout << "• Help maintain code quality and reliability" << std::endl;
    std::cout << "• Reduce debugging time for complex issues" << std::endl;
    
    std::cout << "\nRecommended Usage:" << std::endl;
    std::cout << "• Use ASan + UBSan for general development" << std::endl;
    std::cout << "• Use TSan for multithreaded code" << std::endl;
    std::cout << "• Run sanitizer builds in CI/CD" << std::endl;
    std::cout << "• Create separate build configurations for each sanitizer" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Sanitizers are essential tools for modern C++ development
- Each sanitizer targets specific classes of bugs
- ASan is most commonly used for memory safety
- TSan is crucial for multithreaded applications
- UBSan catches subtle undefined behavior bugs
- Sanitizers should be integrated into development workflow
- Performance overhead is acceptable for testing/debugging
- Combining sanitizers requires understanding compatibility
*/
