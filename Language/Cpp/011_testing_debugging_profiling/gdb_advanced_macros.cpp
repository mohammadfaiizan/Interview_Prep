/*
 * TESTING, DEBUGGING, PROFILING - ADVANCED GDB DEBUGGING
 * 
 * This file demonstrates advanced GDB debugging techniques, custom macros,
 * and debugging strategies for complex C++ applications.
 * 
 * Compilation: g++ -std=c++17 -g -O0 -Wall -Wextra gdb_advanced_macros.cpp -o debug_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>

// =============================================================================
// DEBUGGING HELPER MACROS
// =============================================================================

#ifdef DEBUG
    #define DBG_PRINT(x) std::cout << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " " << x << std::endl
    #define DBG_VAR(var) std::cout << "[DEBUG] " << #var << " = " << var << std::endl
    #define DBG_FUNC() std::cout << "[DEBUG] Entering " << __FUNCTION__ << std::endl
    #define DBG_BREAKPOINT() __builtin_trap()
#else
    #define DBG_PRINT(x)
    #define DBG_VAR(var)
    #define DBG_FUNC()
    #define DBG_BREAKPOINT()
#endif

// Conditional debugging
#define DBG_IF(condition, message) \
    if (condition) { \
        std::cout << "[DEBUG] " << message << std::endl; \
    }

// Memory debugging helpers
#define DBG_NEW(ptr, type) \
    std::cout << "[MEMORY] Allocated " << #type << " at " << ptr << std::endl

#define DBG_DELETE(ptr) \
    std::cout << "[MEMORY] Deallocating " << ptr << std::endl

// =============================================================================
// CLASSES FOR DEBUGGING DEMONSTRATION
// =============================================================================

// Complex data structure for debugging
class DataNode {
private:
    int id_;
    std::string data_;
    std::vector<std::shared_ptr<DataNode>> children_;
    std::weak_ptr<DataNode> parent_;
    
public:
    DataNode(int id, const std::string& data) : id_(id), data_(data) {
        DBG_PRINT("Creating DataNode with id=" << id);
    }
    
    ~DataNode() {
        DBG_PRINT("Destroying DataNode with id=" << id_);
    }
    
    void add_child(std::shared_ptr<DataNode> child) {
        DBG_FUNC();
        children_.push_back(child);
        child->parent_ = shared_from_this();
        DBG_VAR(children_.size());
    }
    
    void set_data(const std::string& data) {
        DBG_PRINT("Changing data from '" << data_ << "' to '" << data << "'");
        data_ = data;
    }
    
    const std::string& get_data() const { return data_; }
    int get_id() const { return id_; }
    size_t child_count() const { return children_.size(); }
    
    void debug_print_tree(int depth = 0) const {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "Node " << id_ << ": " << data_ << std::endl;
        for (const auto& child : children_) {
            child->debug_print_tree(depth + 1);
        }
    }
    
    // Enable shared_from_this
    std::shared_ptr<DataNode> shared_from_this() {
        return std::shared_ptr<DataNode>(this, [](DataNode*){});
    }
};

// Thread-safe counter for debugging race conditions
class ThreadSafeCounter {
private:
    mutable std::mutex mutex_;
    int count_;
    std::string name_;
    
public:
    ThreadSafeCounter(const std::string& name) : count_(0), name_(name) {
        DBG_PRINT("Created counter: " << name);
    }
    
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        DBG_PRINT("Thread " << std::this_thread::get_id() << " incrementing " << name_);
        count_++;
        DBG_VAR(count_);
    }
    
    void decrement() {
        std::lock_guard<std::mutex> lock(mutex_);
        DBG_PRINT("Thread " << std::this_thread::get_id() << " decrementing " << name_);
        count_--;
        DBG_VAR(count_);
    }
    
    int get_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    const std::string& get_name() const { return name_; }
};

// Memory management class for debugging leaks
class MemoryManager {
private:
    std::map<void*, size_t> allocations_;
    mutable std::mutex mutex_;
    
public:
    void* allocate(size_t size) {
        void* ptr = malloc(size);
        std::lock_guard<std::mutex> lock(mutex_);
        allocations_[ptr] = size;
        DBG_PRINT("Allocated " << size << " bytes at " << ptr);
        return ptr;
    }
    
    void deallocate(void* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            DBG_PRINT("Deallocating " << it->second << " bytes at " << ptr);
            allocations_.erase(it);
            free(ptr);
        } else {
            DBG_PRINT("WARNING: Attempting to free untracked pointer " << ptr);
        }
    }
    
    void print_leaks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (allocations_.empty()) {
            std::cout << "No memory leaks detected" << std::endl;
        } else {
            std::cout << "Memory leaks detected:" << std::endl;
            for (const auto& [ptr, size] : allocations_) {
                std::cout << "  " << size << " bytes at " << ptr << std::endl;
            }
        }
    }
    
    size_t get_allocation_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocations_.size();
    }
};

// =============================================================================
// GDB COMMAND DEMONSTRATIONS
// =============================================================================

void demonstrate_gdb_basics() {
    std::cout << "\n=== GDB Basic Commands ===" << std::endl;
    
    std::cout << "\nCompilation for debugging:" << std::endl;
    std::cout << "g++ -g -O0 -std=c++17 program.cpp -o program" << std::endl;
    std::cout << "  -g: Include debug symbols" << std::endl;
    std::cout << "  -O0: Disable optimizations" << std::endl;
    
    std::cout << "\nStarting GDB:" << std::endl;
    std::cout << "gdb ./program                    # Start GDB with program" << std::endl;
    std::cout << "gdb --args ./program arg1 arg2   # Start with arguments" << std::endl;
    std::cout << "gdb -p <pid>                     # Attach to running process" << std::endl;
    
    std::cout << "\nBasic Navigation:" << std::endl;
    std::cout << "run (r)                          # Start program execution" << std::endl;
    std::cout << "continue (c)                     # Continue execution" << std::endl;
    std::cout << "step (s)                         # Step into functions" << std::endl;
    std::cout << "next (n)                         # Step over functions" << std::endl;
    std::cout << "finish                           # Run until function returns" << std::endl;
    std::cout << "quit (q)                         # Exit GDB" << std::endl;
    
    std::cout << "\nBreakpoints:" << std::endl;
    std::cout << "break main                       # Break at main function" << std::endl;
    std::cout << "break file.cpp:25                # Break at line 25" << std::endl;
    std::cout << "break ClassName::method          # Break at method" << std::endl;
    std::cout << "break *0x400123                  # Break at address" << std::endl;
    std::cout << "info breakpoints                 # List all breakpoints" << std::endl;
    std::cout << "delete 1                         # Delete breakpoint 1" << std::endl;
    std::cout << "disable 2                        # Disable breakpoint 2" << std::endl;
}

void demonstrate_advanced_breakpoints() {
    std::cout << "\n=== Advanced Breakpoint Techniques ===" << std::endl;
    
    std::cout << "\nConditional Breakpoints:" << std::endl;
    std::cout << "break main if argc > 1           # Break only if condition true" << std::endl;
    std::cout << "break loop.cpp:10 if i == 100    # Break when variable equals value" << std::endl;
    std::cout << "condition 1 x > 50               # Add condition to existing breakpoint" << std::endl;
    
    std::cout << "\nWatchpoints (Data Breakpoints):" << std::endl;
    std::cout << "watch variable                   # Break when variable changes" << std::endl;
    std::cout << "rwatch variable                  # Break when variable is read" << std::endl;
    std::cout << "awatch variable                  # Break when variable is accessed" << std::endl;
    std::cout << "watch *(int*)0x12345678          # Watch memory location" << std::endl;
    
    std::cout << "\nTemporary and Counted Breakpoints:" << std::endl;
    std::cout << "tbreak main                      # Temporary breakpoint (auto-delete)" << std::endl;
    std::cout << "ignore 1 10                      # Ignore breakpoint 1 for 10 hits" << std::endl;
    std::cout << "commands 1                       # Add commands to breakpoint 1" << std::endl;
    std::cout << "  print variable" << std::endl;
    std::cout << "  continue" << std::endl;
    std::cout << "end" << std::endl;
    
    std::cout << "\nFunction and Method Breakpoints:" << std::endl;
    std::cout << "break 'std::vector<int>::push_back(int const&)'" << std::endl;
    std::cout << "break 'operator new(unsigned long)'" << std::endl;
    std::cout << "rbreak regex                     # Break on functions matching regex" << std::endl;
}

void demonstrate_data_examination() {
    std::cout << "\n=== Data Examination Commands ===" << std::endl;
    
    std::cout << "\nVariable Inspection:" << std::endl;
    std::cout << "print variable (p)               # Print variable value" << std::endl;
    std::cout << "print/x variable                 # Print in hexadecimal" << std::endl;
    std::cout << "print/t variable                 # Print in binary" << std::endl;
    std::cout << "print/c variable                 # Print as character" << std::endl;
    std::cout << "print sizeof(variable)           # Print size of variable" << std::endl;
    
    std::cout << "\nMemory Examination:" << std::endl;
    std::cout << "x/10i $pc                        # Examine 10 instructions at PC" << std::endl;
    std::cout << "x/20x address                    # Examine 20 hex words" << std::endl;
    std::cout << "x/s string_ptr                   # Examine as string" << std::endl;
    std::cout << "x/10gx array                     # Examine as 64-bit hex values" << std::endl;
    
    std::cout << "\nComplex Data Structures:" << std::endl;
    std::cout << "print *pointer                   # Dereference pointer" << std::endl;
    std::cout << "print array[5]                   # Array element" << std::endl;
    std::cout << "print object.member              # Object member" << std::endl;
    std::cout << "print object->member             # Pointer to object member" << std::endl;
    std::cout << "print vector._M_impl._M_start    # STL container internals" << std::endl;
    
    std::cout << "\nType Information:" << std::endl;
    std::cout << "whatis variable                  # Show variable type" << std::endl;
    std::cout << "ptype ClassName                  # Show class definition" << std::endl;
    std::cout << "info variables                   # List all variables" << std::endl;
    std::cout << "info locals                      # List local variables" << std::endl;
    std::cout << "info args                        # List function arguments" << std::endl;
}

void demonstrate_stack_analysis() {
    std::cout << "\n=== Stack and Frame Analysis ===" << std::endl;
    
    std::cout << "\nStack Traces:" << std::endl;
    std::cout << "backtrace (bt)                   # Show call stack" << std::endl;
    std::cout << "bt full                          # Show stack with local variables" << std::endl;
    std::cout << "bt 10                            # Show top 10 frames" << std::endl;
    std::cout << "thread apply all bt              # Stack trace for all threads" << std::endl;
    
    std::cout << "\nFrame Navigation:" << std::endl;
    std::cout << "frame 2 (f 2)                    # Switch to frame 2" << std::endl;
    std::cout << "up                               # Move up one frame" << std::endl;
    std::cout << "down                             # Move down one frame" << std::endl;
    std::cout << "info frame                       # Show current frame info" << std::endl;
    
    std::cout << "\nRegister Examination:" << std::endl;
    std::cout << "info registers                   # Show all registers" << std::endl;
    std::cout << "print $rax                       # Print specific register" << std::endl;
    std::cout << "print $pc                        # Print program counter" << std::endl;
    std::cout << "print $sp                        # Print stack pointer" << std::endl;
}

void demonstrate_multithreading_debug() {
    std::cout << "\n=== Multithreading Debugging ===" << std::endl;
    
    std::cout << "\nThread Management:" << std::endl;
    std::cout << "info threads                     # List all threads" << std::endl;
    std::cout << "thread 2                         # Switch to thread 2" << std::endl;
    std::cout << "thread apply all command         # Run command on all threads" << std::endl;
    std::cout << "thread apply 1-3 bt              # Backtrace for threads 1-3" << std::endl;
    
    std::cout << "\nThread-Specific Breakpoints:" << std::endl;
    std::cout << "break file.cpp:25 thread 2       # Break only in thread 2" << std::endl;
    std::cout << "break function thread 2 if x>10  # Conditional thread breakpoint" << std::endl;
    
    std::cout << "\nScheduler Control:" << std::endl;
    std::cout << "set scheduler-locking on         # Lock scheduler to current thread" << std::endl;
    std::cout << "set scheduler-locking step        # Lock during stepping only" << std::endl;
    std::cout << "set scheduler-locking off         # Normal scheduling" << std::endl;
    
    std::cout << "\nDeadlock Detection:" << std::endl;
    std::cout << "thread apply all bt              # Check all thread stacks" << std::endl;
    std::cout << "info threads                     # Look for waiting threads" << std::endl;
    std::cout << "print mutex_object               # Examine mutex state" << std::endl;
}

void demonstrate_custom_gdb_commands() {
    std::cout << "\n=== Custom GDB Commands and Macros ===" << std::endl;
    
    std::cout << "\nGDB Script Example (.gdbinit or script file):" << std::endl;
    std::cout << R"(
# Custom command to print vector contents
define pvector
    if $argc == 0
        help pvector
    else
        set $size = $arg0.size()
        set $capacity = $arg0.capacity()
        set $i = 0
        printf "Vector size: %d, capacity: %d\n", $size, $capacity
        while $i < $size
            printf "[%d] = ", $i
            print $arg0[$i]
            set $i = $i + 1
        end
    end
end

document pvector
Print contents of std::vector
Usage: pvector vector_name
end

# Custom command to print map contents
define pmap
    set $i = 0
    set $node = $arg0._M_t._M_impl._M_header._M_left
    printf "Map contents:\n"
    while $node != $arg0._M_t._M_impl._M_header
        printf "[%d] key: ", $i
        print $node._M_value_field.first
        printf "    value: "
        print $node._M_value_field.second
        set $node = $node._M_right
        set $i = $i + 1
    end
end

# Convenience aliases
alias -a ll = list
alias -a bb = break
alias -a cc = continue
alias -a nn = next
alias -a ss = step

# Pretty printing settings
set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
)" << std::endl;

    std::cout << "\nPython Scripting in GDB:" << std::endl;
    std::cout << R"(
# Python script for custom pretty printers
python
import gdb

class VectorPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = int(self.val['_M_impl']['_M_finish'] - self.val['_M_impl']['_M_start'])
        return f"std::vector of length {size}"

    def children(self):
        start = self.val['_M_impl']['_M_start']
        finish = self.val['_M_impl']['_M_finish']
        for i in range(int(finish - start)):
            yield f'[{i}]', start[i]

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("custom")
    pp.add_printer('vector', '^std::vector<.*>$', VectorPrinter)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())
end
)" << std::endl;
}

// =============================================================================
// PRACTICAL DEBUGGING SCENARIOS
// =============================================================================

void demonstrate_debugging_scenarios() {
    std::cout << "\n=== Common Debugging Scenarios ===" << std::endl;
    
    // Scenario 1: Memory corruption
    std::cout << "\n1. Memory Corruption Detection:" << std::endl;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    DBG_PRINT("Vector created with " << vec.size() << " elements");
    
    // Intentional bug for demonstration
    // vec[10] = 42;  // This would cause undefined behavior
    
    std::cout << "GDB commands for memory issues:" << std::endl;
    std::cout << "  watch vec._M_impl._M_start     # Watch vector data pointer" << std::endl;
    std::cout << "  x/10x vec._M_impl._M_start     # Examine vector memory" << std::endl;
    std::cout << "  print vec.size()               # Check vector size" << std::endl;
    
    // Scenario 2: Infinite loop debugging
    std::cout << "\n2. Infinite Loop Detection:" << std::endl;
    int counter = 0;
    DBG_PRINT("Starting loop with counter=" << counter);
    
    // Simulate a loop that could be infinite
    while (counter < 5) {
        DBG_VAR(counter);
        counter++;
        // In real debugging: Ctrl+C to break, then examine variables
    }
    
    std::cout << "GDB commands for infinite loops:" << std::endl;
    std::cout << "  Ctrl+C                         # Interrupt execution" << std::endl;
    std::cout << "  print counter                  # Check loop variable" << std::endl;
    std::cout << "  watch counter                  # Watch variable changes" << std::endl;
    
    // Scenario 3: Null pointer debugging
    std::cout << "\n3. Null Pointer Detection:" << std::endl;
    std::shared_ptr<DataNode> node_ptr = std::make_shared<DataNode>(1, "test");
    DBG_PRINT("Created node pointer: " << node_ptr.get());
    
    if (node_ptr) {
        DBG_PRINT("Node data: " << node_ptr->get_data());
    }
    
    std::cout << "GDB commands for null pointers:" << std::endl;
    std::cout << "  print pointer                  # Check pointer value" << std::endl;
    std::cout << "  print *pointer                 # Dereference (will crash if null)" << std::endl;
    std::cout << "  watch pointer                  # Watch for pointer changes" << std::endl;
}

void demonstrate_performance_debugging() {
    std::cout << "\n=== Performance Debugging with GDB ===" << std::endl;
    
    std::cout << "\nProfiling with GDB:" << std::endl;
    std::cout << "1. Statistical Profiling:" << std::endl;
    std::cout << "   - Run program under GDB" << std::endl;
    std::cout << "   - Interrupt frequently with Ctrl+C" << std::endl;
    std::cout << "   - Note where execution stops most often" << std::endl;
    
    std::cout << "\n2. Function Call Tracing:" << std::endl;
    std::cout << R"(
# Trace all function calls
set trace-commands on
break function_name
commands
  bt 1
  continue
end
)" << std::endl;

    std::cout << "\n3. Memory Usage Analysis:" << std::endl;
    std::cout << "   - Use custom memory manager class" << std::endl;
    std::cout << "   - Set breakpoints on malloc/free" << std::endl;
    std::cout << "   - Track allocation patterns" << std::endl;
    
    // Demonstrate with actual code
    MemoryManager mem_mgr;
    void* ptr1 = mem_mgr.allocate(100);
    void* ptr2 = mem_mgr.allocate(200);
    
    DBG_PRINT("Allocated memory blocks: " << mem_mgr.get_allocation_count());
    
    mem_mgr.deallocate(ptr1);
    DBG_PRINT("After deallocation: " << mem_mgr.get_allocation_count());
    
    // Intentionally not freeing ptr2 to show leak detection
    mem_mgr.print_leaks();
    
    mem_mgr.deallocate(ptr2);  // Clean up
}

void demonstrate_crash_analysis() {
    std::cout << "\n=== Crash Analysis Techniques ===" << std::endl;
    
    std::cout << "\nCore Dump Analysis:" << std::endl;
    std::cout << "1. Enable core dumps:" << std::endl;
    std::cout << "   ulimit -c unlimited" << std::endl;
    std::cout << "   echo core > /proc/sys/kernel/core_pattern" << std::endl;
    
    std::cout << "\n2. Analyze core dump:" << std::endl;
    std::cout << "   gdb program core" << std::endl;
    std::cout << "   bt                             # Show crash stack trace" << std::endl;
    std::cout << "   info registers                 # Show register state at crash" << std::endl;
    std::cout << "   print variable                 # Examine variables at crash" << std::endl;
    
    std::cout << "\n3. Post-mortem debugging commands:" << std::endl;
    std::cout << "   where                          # Show crash location" << std::endl;
    std::cout << "   list                           # Show source around crash" << std::endl;
    std::cout << "   info locals                    # Show local variables" << std::endl;
    std::cout << "   up/down                        # Navigate call stack" << std::endl;
    
    std::cout << "\nCommon Crash Patterns:" << std::endl;
    std::cout << "• Segmentation Fault: Invalid memory access" << std::endl;
    std::cout << "• Abort: Failed assertion or explicit abort()" << std::endl;
    std::cout << "• Stack Overflow: Infinite recursion" << std::endl;
    std::cout << "• Double Free: Freeing same memory twice" << std::endl;
    std::cout << "• Use After Free: Accessing freed memory" << std::endl;
}

int main() {
    std::cout << "ADVANCED GDB DEBUGGING TECHNIQUES\n";
    std::cout << "==================================\n";
    
    demonstrate_gdb_basics();
    demonstrate_advanced_breakpoints();
    demonstrate_data_examination();
    demonstrate_stack_analysis();
    demonstrate_multithreading_debug();
    demonstrate_custom_gdb_commands();
    demonstrate_debugging_scenarios();
    demonstrate_performance_debugging();
    demonstrate_crash_analysis();
    
    std::cout << "\nKey GDB Concepts:" << std::endl;
    std::cout << "• GDB provides powerful debugging capabilities for C++" << std::endl;
    std::cout << "• Compile with -g -O0 for best debugging experience" << std::endl;
    std::cout << "• Breakpoints, watchpoints, and conditional breaks are essential" << std::endl;
    std::cout << "• Custom commands and scripts enhance productivity" << std::endl;
    std::cout << "• Thread debugging requires special consideration" << std::endl;
    std::cout << "• Core dumps enable post-mortem analysis" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- GDB is an essential tool for C++ debugging
- Proper compilation flags (-g -O0) are crucial for debugging
- Advanced breakpoint techniques save significant debugging time
- Custom GDB commands and scripts improve productivity
- Understanding assembly and memory layout helps with complex bugs
- Multithreaded debugging requires special techniques
- Core dump analysis enables post-mortem debugging
- Integration with IDEs provides visual debugging interfaces
*/
