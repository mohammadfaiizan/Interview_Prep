/*
 * CMAKE AND TOOLCHAIN - STATIC VS DYNAMIC LINKING
 * 
 * This file demonstrates static and dynamic linking concepts
 * with practical C++ examples and CMake configurations.
 * 
 * Compilation examples provided below
 */

#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>  // For dynamic loading example
#include <chrono>

// =============================================================================
// LIBRARY EXAMPLES FOR LINKING DEMONSTRATION
// =============================================================================

// Static library example
class StaticMathLibrary {
public:
    static double add(double a, double b) { return a + b; }
    static double multiply(double a, double b) { return a * b; }
    static std::string get_version() { return "StaticMath v1.0"; }
    
    static void demonstrate_static_features() {
        std::cout << "Static library features:" << std::endl;
        std::cout << "- Compiled into executable" << std::endl;
        std::cout << "- No runtime dependencies" << std::endl;
        std::cout << "- Larger executable size" << std::endl;
        std::cout << "- Faster function calls (no indirection)" << std::endl;
    }
};

// Shared library example interface
class SharedUtilsLibrary {
public:
    static std::string process_string(const std::string& input) {
        return "Processed: " + input;
    }
    
    static std::vector<int> generate_sequence(int count) {
        std::vector<int> result;
        for (int i = 0; i < count; ++i) {
            result.push_back(i * i);
        }
        return result;
    }
    
    static std::string get_version() { return "SharedUtils v2.1"; }
    
    static void demonstrate_shared_features() {
        std::cout << "Shared library features:" << std::endl;
        std::cout << "- Loaded at runtime" << std::endl;
        std::cout << "- Shared between processes" << std::endl;
        std::cout << "- Smaller executable size" << std::endl;
        std::cout << "- Updates without recompilation" << std::endl;
    }
};

// =============================================================================
// STATIC LINKING DEMONSTRATION
// =============================================================================

void demonstrate_static_linking() {
    std::cout << "\n=== Static Linking Demonstration ===" << std::endl;
    
    // Use static library
    StaticMathLibrary::demonstrate_static_features();
    
    std::cout << "\nUsing static math library:" << std::endl;
    std::cout << "Version: " << StaticMathLibrary::get_version() << std::endl;
    std::cout << "5 + 3 = " << StaticMathLibrary::add(5, 3) << std::endl;
    std::cout << "5 * 3 = " << StaticMathLibrary::multiply(5, 3) << std::endl;
    
    std::cout << "\nStatic Linking CMake Example:" << std::endl;
    std::cout << R"(
# Create static library
add_library(math_static STATIC
    src/math.cpp
    include/math.h)

target_include_directories(math_static PUBLIC include/)

# Link static library to executable
add_executable(static_app main.cpp)
target_link_libraries(static_app PRIVATE math_static)

# Force static linking even if shared exists
set_target_properties(math_static PROPERTIES
    POSITION_INDEPENDENT_CODE OFF)
)" << std::endl;

    std::cout << "\nBuild commands:" << std::endl;
    std::cout << "ar rcs libmath.a math.o          # Create static archive" << std::endl;
    std::cout << "g++ main.cpp -L. -lmath -o app   # Link static library" << std::endl;
}

void demonstrate_dynamic_linking() {
    std::cout << "\n=== Dynamic Linking Demonstration ===" << std::endl;
    
    // Use shared library
    SharedUtilsLibrary::demonstrate_shared_features();
    
    std::cout << "\nUsing shared utils library:" << std::endl;
    std::cout << "Version: " << SharedUtilsLibrary::get_version() << std::endl;
    std::cout << SharedUtilsLibrary::process_string("Hello World") << std::endl;
    
    auto sequence = SharedUtilsLibrary::generate_sequence(5);
    std::cout << "Generated sequence: ";
    for (int val : sequence) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\nDynamic Linking CMake Example:" << std::endl;
    std::cout << R"(
# Create shared library
add_library(utils_shared SHARED
    src/utils.cpp
    include/utils.h)

target_include_directories(utils_shared PUBLIC include/)

# Set version information
set_target_properties(utils_shared PROPERTIES
    VERSION 2.1.0
    SOVERSION 2
    POSITION_INDEPENDENT_CODE ON)

# Link shared library to executable
add_executable(dynamic_app main.cpp)
target_link_libraries(dynamic_app PRIVATE utils_shared)
)" << std::endl;

    std::cout << "\nBuild commands:" << std::endl;
    std::cout << "g++ -shared -fPIC utils.cpp -o libutils.so  # Create shared library" << std::endl;
    std::cout << "g++ main.cpp -L. -lutils -o app             # Link shared library" << std::endl;
    std::cout << "LD_LIBRARY_PATH=. ./app                      # Run with library path" << std::endl;
}

// =============================================================================
// DYNAMIC LOADING DEMONSTRATION
// =============================================================================

void demonstrate_dynamic_loading() {
    std::cout << "\n=== Dynamic Loading (dlopen) Demonstration ===" << std::endl;
    
    std::cout << "Dynamic loading allows loading libraries at runtime:" << std::endl;
    
    std::cout << "\nExample dynamic loading code:" << std::endl;
    std::cout << R"(
#include <dlfcn.h>

// Function pointer type
typedef int (*add_func_t)(int, int);

void load_and_use_library() {
    // Load library at runtime
    void* handle = dlopen("./libmath.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot load library: " << dlerror() << std::endl;
        return;
    }
    
    // Get function pointer
    add_func_t add_func = (add_func_t) dlsym(handle, "add");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol: " << dlsym_error << std::endl;
        dlclose(handle);
        return;
    }
    
    // Use function
    int result = add_func(5, 3);
    std::cout << "5 + 3 = " << result << std::endl;
    
    // Unload library
    dlclose(handle);
}
)" << std::endl;

    std::cout << "\nDynamic Loading CMake Example:" << std::endl;
    std::cout << R"(
# Create plugin library
add_library(math_plugin SHARED
    src/math_plugin.cpp)

# Don't prefix with 'lib' for plugins
set_target_properties(math_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so")

# Main application with dynamic loading
add_executable(plugin_app main.cpp)
target_link_libraries(plugin_app PRIVATE ${CMAKE_DL_LIBS})
)" << std::endl;

    std::cout << "\nUse cases for dynamic loading:" << std::endl;
    std::cout << "- Plugin systems" << std::endl;
    std::cout << "- Optional features" << std::endl;
    std::cout << "- License-specific modules" << std::endl;
    std::cout << "- Runtime feature detection" << std::endl;
}

// =============================================================================
// LINKING COMPARISON AND PERFORMANCE
// =============================================================================

void demonstrate_linking_comparison() {
    std::cout << "\n=== Static vs Dynamic Linking Comparison ===" << std::endl;
    
    std::cout << "┌─────────────────┬──────────────────┬──────────────────┐" << std::endl;
    std::cout << "│ Aspect          │ Static Linking   │ Dynamic Linking  │" << std::endl;
    std::cout << "├─────────────────┼──────────────────┼──────────────────┤" << std::endl;
    std::cout << "│ Executable Size │ Larger           │ Smaller          │" << std::endl;
    std::cout << "│ Memory Usage    │ Higher           │ Lower (shared)   │" << std::endl;
    std::cout << "│ Load Time       │ Faster           │ Slower           │" << std::endl;
    std::cout << "│ Runtime Deps    │ None             │ Required         │" << std::endl;
    std::cout << "│ Updates         │ Recompile needed │ Library only     │" << std::endl;
    std::cout << "│ Performance     │ Slightly faster  │ Slightly slower  │" << std::endl;
    std::cout << "│ Distribution    │ Self-contained   │ Multiple files   │" << std::endl;
    std::cout << "│ Version Control │ Fixed at build   │ Runtime flexible │" << std::endl;
    std::cout << "└─────────────────┴──────────────────┴──────────────────┘" << std::endl;
    
    // Simple performance comparison
    auto start = std::chrono::high_resolution_clock::now();
    
    // Static library calls
    for (int i = 0; i < 1000000; ++i) {
        volatile double result = StaticMathLibrary::add(i, i + 1);
        (void)result; // Suppress unused variable warning
    }
    
    auto static_time = std::chrono::high_resolution_clock::now() - start;
    
    start = std::chrono::high_resolution_clock::now();
    
    // Shared library calls
    for (int i = 0; i < 1000000; ++i) {
        volatile std::string result = SharedUtilsLibrary::process_string("test");
        (void)result; // Suppress unused variable warning
    }
    
    auto shared_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "\nPerformance comparison (1M calls):" << std::endl;
    std::cout << "Static library: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(static_time).count() 
              << " μs" << std::endl;
    std::cout << "Shared library: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(shared_time).count() 
              << " μs" << std::endl;
}

// =============================================================================
// CMAKE LINKING CONFIGURATION
// =============================================================================

void demonstrate_cmake_linking_options() {
    std::cout << "\n=== CMake Linking Configuration ===" << std::endl;
    
    std::cout << "\n1. Global Linking Preference:" << std::endl;
    std::cout << R"(
# Prefer static libraries globally
set(BUILD_SHARED_LIBS OFF)

# Or prefer shared libraries globally
set(BUILD_SHARED_LIBS ON)

# Control per-target
add_library(mylib_static STATIC src.cpp)
add_library(mylib_shared SHARED src.cpp)
)" << std::endl;

    std::cout << "\n2. Link Type Control:" << std::endl;
    std::cout << R"(
# Force static linking
target_link_libraries(my_app PRIVATE -static)

# Prefer static libraries
set_target_properties(my_app PROPERTIES
    LINK_SEARCH_START_STATIC ON
    LINK_SEARCH_END_STATIC ON)

# Mixed linking
target_link_libraries(my_app PRIVATE
    mylib_static        # Always static
    -Wl,-Bstatic pthread  # Force static pthread
    -Wl,-Bdynamic dl     # Force dynamic dl
)
)" << std::endl;

    std::cout << "\n3. Runtime Path Configuration:" << std::endl;
    std::cout << R"(
# Set RPATH for shared libraries
set_target_properties(my_app PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE)

# Skip RPATH (use LD_LIBRARY_PATH)
set_target_properties(my_app PROPERTIES
    SKIP_BUILD_RPATH TRUE
    SKIP_INSTALL_RPATH TRUE)

# Relative RPATH
set_target_properties(my_app PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib")
)" << std::endl;

    std::cout << "\n4. Position Independent Code:" << std::endl;
    std::cout << R"(
# Enable PIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Per-target PIC control
set_target_properties(mylib PROPERTIES
    POSITION_INDEPENDENT_CODE ON)

# Compiler-specific PIC flags
target_compile_options(mylib PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fPIC>)
)" << std::endl;
}

void demonstrate_troubleshooting() {
    std::cout << "\n=== Linking Troubleshooting ===" << std::endl;
    
    std::cout << "\nCommon linking issues and solutions:" << std::endl;
    
    std::cout << "\n1. Undefined symbols:" << std::endl;
    std::cout << "   Problem: Missing library or wrong order" << std::endl;
    std::cout << "   Solution: Check library dependencies and link order" << std::endl;
    std::cout << "   Tools: nm, objdump, ldd" << std::endl;
    
    std::cout << "\n2. Library not found:" << std::endl;
    std::cout << "   Problem: Library not in search path" << std::endl;
    std::cout << "   Solution: Add library directory with -L or LIBRARY_PATH" << std::endl;
    std::cout << "   CMake: target_link_directories()" << std::endl;
    
    std::cout << "\n3. Version conflicts:" << std::endl;
    std::cout << "   Problem: Multiple versions of same library" << std::endl;
    std::cout << "   Solution: Use specific versions or namespaces" << std::endl;
    std::cout << "   CMake: Use imported targets with version checks" << std::endl;
    
    std::cout << "\n4. Runtime loading issues:" << std::endl;
    std::cout << "   Problem: Shared library not found at runtime" << std::endl;
    std::cout << "   Solution: Set LD_LIBRARY_PATH or use RPATH" << std::endl;
    std::cout << "   Tools: ldd, readelf -d" << std::endl;
    
    std::cout << "\nUseful debugging commands:" << std::endl;
    std::cout << "ldd ./my_app                    # Show shared library dependencies" << std::endl;
    std::cout << "nm -D libmylib.so              # Show dynamic symbols" << std::endl;
    std::cout << "objdump -t libmylib.a          # Show static symbols" << std::endl;
    std::cout << "readelf -d ./my_app            # Show dynamic section" << std::endl;
    std::cout << "ldconfig -p | grep mylib       # Find library in system" << std::endl;
}

int main() {
    std::cout << "STATIC VS DYNAMIC LINKING\n";
    std::cout << "=========================\n";
    
    demonstrate_static_linking();
    demonstrate_dynamic_linking();
    demonstrate_dynamic_loading();
    demonstrate_linking_comparison();
    demonstrate_cmake_linking_options();
    demonstrate_troubleshooting();
    
    std::cout << "\nKey Linking Concepts:" << std::endl;
    std::cout << "- Static linking embeds libraries in executable" << std::endl;
    std::cout << "- Dynamic linking loads libraries at runtime" << std::endl;
    std::cout << "- Dynamic loading allows runtime library selection" << std::endl;
    std::cout << "- Choose based on deployment and performance needs" << std::endl;
    std::cout << "- CMake provides flexible linking control" << std::endl;
    std::cout << "- RPATH manages runtime library paths" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Static linking creates self-contained executables
- Dynamic linking enables code sharing and updates
- Dynamic loading provides maximum flexibility
- Performance differences are usually minimal
- Memory usage varies with deployment scenario
- CMake abstracts platform-specific linking details
- Proper RPATH configuration is crucial for deployment
- Debugging tools help resolve linking issues
*/
