/*
 * CMAKE AND TOOLCHAIN - TARGETS AND TARGET PROPERTIES
 * 
 * This file demonstrates CMake target concepts and properties
 * through practical C++ examples.
 * 
 * Compilation: See CMake target examples below
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

// =============================================================================
// TARGET TYPES DEMONSTRATION
// =============================================================================

// Example library for static/shared target demonstration
class TargetLibrary {
public:
    static std::string get_library_info() {
        return "Target Library v1.0 - Demonstrating CMake targets";
    }
    
    static int perform_calculation(int a, int b) {
        return a * b + (a + b);
    }
    
    static void simulate_work() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

// Example interface for header-only library
template<typename T>
class InterfaceLibrary {
public:
    static T max(T a, T b) { return (a > b) ? a : b; }
    static T min(T a, T b) { return (a < b) ? a : b; }
    
    static std::vector<T> sort_vector(std::vector<T> vec) {
        std::sort(vec.begin(), vec.end());
        return vec;
    }
};

// Example application class
class Application {
private:
    std::string name_;
    std::string version_;
    
public:
    Application(const std::string& name, const std::string& version)
        : name_(name), version_(version) {}
    
    void run() {
        std::cout << "Running " << name_ << " v" << version_ << std::endl;
        
        // Use library functionality
        std::cout << TargetLibrary::get_library_info() << std::endl;
        
        int result = TargetLibrary::perform_calculation(5, 3);
        std::cout << "Calculation result: " << result << std::endl;
        
        // Use template library
        auto max_val = InterfaceLibrary<int>::max(10, 20);
        std::cout << "Max value: " << max_val << std::endl;
        
        std::vector<int> numbers = {3, 1, 4, 1, 5, 9, 2, 6};
        auto sorted = InterfaceLibrary<int>::sort_vector(numbers);
        std::cout << "Sorted numbers: ";
        for (int n : sorted) {
            std::cout << n << " ";
        }
        std::cout << std::endl;
    }
};

// =============================================================================
// CMAKE TARGET EXAMPLES
// =============================================================================

void demonstrate_target_types() {
    std::cout << "\n=== CMake Target Types ===" << std::endl;
    
    std::cout << "\n1. Executable Targets:" << std::endl;
    std::cout << R"(
# Basic executable
add_executable(my_app main.cpp)

# Executable with multiple sources
add_executable(complex_app
    main.cpp
    app.cpp
    utils.cpp
    config.cpp)

# Executable with WIN32 property (Windows GUI app)
add_executable(gui_app WIN32 main.cpp gui.cpp)

# Executable with MACOSX_BUNDLE property (macOS app bundle)
add_executable(mac_app MACOSX_BUNDLE main.cpp)
)" << std::endl;

    std::cout << "\n2. Library Targets:" << std::endl;
    std::cout << R"(
# Static library (default)
add_library(mylib_static STATIC 
    lib.cpp 
    lib.h)

# Shared library
add_library(mylib_shared SHARED 
    lib.cpp 
    lib.h)

# Object library (compiled objects, not archived)
add_library(mylib_objects OBJECT 
    lib.cpp 
    lib.h)

# Interface library (header-only)
add_library(mylib_interface INTERFACE)

# Module library (plugin/loadable module)
add_library(mylib_module MODULE 
    plugin.cpp 
    plugin.h)
)" << std::endl;

    std::cout << "\n3. Imported Targets:" << std::endl;
    std::cout << R"(
# Imported executable
add_executable(external_tool IMPORTED)
set_target_properties(external_tool PROPERTIES
    IMPORTED_LOCATION "/usr/bin/tool")

# Imported library
add_library(external_lib SHARED IMPORTED)
set_target_properties(external_lib PROPERTIES
    IMPORTED_LOCATION "/usr/lib/libexternal.so"
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/external")

# Alias target (creates alias for existing target)
add_library(MyProject::mylib ALIAS mylib_static)
)" << std::endl;
}

void demonstrate_target_properties() {
    std::cout << "\n=== Target Properties ===" << std::endl;
    
    std::cout << "\nCommon Target Properties:" << std::endl;
    std::cout << R"(
# Output name and location
set_target_properties(my_app PROPERTIES
    OUTPUT_NAME "application"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Version information
set_target_properties(mylib PROPERTIES
    VERSION 1.2.3
    SOVERSION 1)

# C++ standard
set_target_properties(my_app PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# Compile definitions
set_target_properties(my_app PROPERTIES
    COMPILE_DEFINITIONS "MY_DEFINE=1;ANOTHER_DEFINE")

# Include directories
set_target_properties(mylib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>")
)" << std::endl;

    std::cout << "\nPlatform-specific Properties:" << std::endl;
    std::cout << R"(
# Windows-specific
set_target_properties(my_app PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/SUBSYSTEM:WINDOWS")

# macOS-specific
set_target_properties(my_app PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist")

# Position Independent Code
set_target_properties(mylib PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
)" << std::endl;
}

void demonstrate_target_commands() {
    std::cout << "\n=== Modern Target Commands ===" << std::endl;
    
    std::cout << "\n1. target_link_libraries():" << std::endl;
    std::cout << R"(
# Link with visibility control
target_link_libraries(my_app 
    PRIVATE mylib_internal      # Only for building my_app
    PUBLIC mylib_api           # For my_app and its consumers
    INTERFACE mylib_header_only # Only for consumers of my_app
)

# Link system libraries
target_link_libraries(my_app PRIVATE 
    Threads::Threads
    ${CMAKE_DL_LIBS}
    m)  # math library on Unix

# Link with generator expressions
target_link_libraries(my_app PRIVATE
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Linux>:pthread>)
)" << std::endl;

    std::cout << "\n2. target_include_directories():" << std::endl;
    std::cout << R"(
# Include directories with visibility
target_include_directories(mylib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src     # Internal headers
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>        # For installed package
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/interface  # Header-only interface
)

# System include directories (no warnings)
target_include_directories(my_app SYSTEM PRIVATE
    ${EXTERNAL_INCLUDE_DIR})
)" << std::endl;

    std::cout << "\n3. target_compile_definitions():" << std::endl;
    std::cout << R"(
# Compile definitions with visibility
target_compile_definitions(mylib
    PRIVATE
        INTERNAL_DEBUG=1
        $<$<CONFIG:Debug>:DEBUG_BUILD>
    PUBLIC
        MYLIB_VERSION_MAJOR=1
        MYLIB_VERSION_MINOR=0
    INTERFACE
        MYLIB_HEADER_ONLY
)

# Platform-specific definitions
target_compile_definitions(my_app PRIVATE
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
    $<$<PLATFORM_ID:Linux>:_GNU_SOURCE>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
)" << std::endl;

    std::cout << "\n4. target_compile_options():" << std::endl;
    std::cout << R"(
# Compiler options with visibility
target_compile_options(mylib
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        $<$<CXX_COMPILER_ID:Clang>:-Weverything>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
    PUBLIC
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O3>
)

# Architecture-specific options
target_compile_options(my_app PRIVATE
    $<$<PLATFORM_ID:Linux>:-march=native>
    $<$<PLATFORM_ID:Windows>:/arch:AVX2>)
)" << std::endl;

    std::cout << "\n5. target_compile_features():" << std::endl;
    std::cout << R"(
# C++ features
target_compile_features(mylib PUBLIC
    cxx_std_17              # Require C++17
    cxx_constexpr           # Require constexpr support
    cxx_auto_type           # Require auto type deduction
)

# C features
target_compile_features(c_lib PUBLIC
    c_std_11                # Require C11
    c_static_assert         # Require static_assert
)
)" << std::endl;
}

void demonstrate_generator_expressions() {
    std::cout << "\n=== Generator Expressions ===" << std::endl;
    
    std::cout << "\nGenerator expressions enable conditional logic at build time:" << std::endl;
    
    std::cout << "\n1. Configuration-based:" << std::endl;
    std::cout << "$<CONFIG:Debug>                    # True for Debug builds" << std::endl;
    std::cout << "$<CONFIG:Release>                  # True for Release builds" << std::endl;
    std::cout << "$<$<CONFIG:Debug>:DEBUG_FLAG>      # Include DEBUG_FLAG in Debug" << std::endl;
    
    std::cout << "\n2. Platform-based:" << std::endl;
    std::cout << "$<PLATFORM_ID:Windows>             # True on Windows" << std::endl;
    std::cout << "$<PLATFORM_ID:Linux>               # True on Linux" << std::endl;
    std::cout << "$<$<PLATFORM_ID:Windows>:kernel32> # Link kernel32 on Windows" << std::endl;
    
    std::cout << "\n3. Compiler-based:" << std::endl;
    std::cout << "$<CXX_COMPILER_ID:GNU>             # True for GCC" << std::endl;
    std::cout << "$<CXX_COMPILER_ID:Clang>           # True for Clang" << std::endl;
    std::cout << "$<CXX_COMPILER_ID:MSVC>            # True for MSVC" << std::endl;
    
    std::cout << "\n4. Target-based:" << std::endl;
    std::cout << "$<TARGET_EXISTS:mylib>             # True if target exists" << std::endl;
    std::cout << "$<TARGET_PROPERTY:mylib,TYPE>      # Get target type" << std::endl;
    
    std::cout << "\n5. Build interface vs Install interface:" << std::endl;
    std::cout << "$<BUILD_INTERFACE:build/include>   # Used during build" << std::endl;
    std::cout << "$<INSTALL_INTERFACE:include>       # Used after install" << std::endl;
    
    std::cout << "\nPractical Examples:" << std::endl;
    std::cout << R"(
# Conditional linking
target_link_libraries(my_app PRIVATE
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Linux>:pthread>
    $<$<CONFIG:Debug>:debug_lib>)

# Conditional compile definitions
target_compile_definitions(my_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE=1>
    $<$<CONFIG:Release>:NDEBUG>
    VERSION="${PROJECT_VERSION}")

# Conditional include directories
target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
)" << std::endl;
}

void demonstrate_target_dependencies() {
    std::cout << "\n=== Target Dependencies ===" << std::endl;
    
    std::cout << "\nDependency Types:" << std::endl;
    std::cout << R"(
# Build dependencies (compile-time)
add_dependencies(my_app generate_headers)

# Link dependencies (link-time and runtime)
target_link_libraries(my_app PRIVATE mylib)

# Custom target dependencies
add_custom_target(documentation
    COMMAND doxygen Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation")

add_dependencies(my_app documentation)
)" << std::endl;

    std::cout << "\nTransitive Dependencies:" << std::endl;
    std::cout << R"(
# Library A depends on Library B
target_link_libraries(libA PRIVATE libB)

# Application depends on Library A
# Library B is automatically linked (if PUBLIC/INTERFACE)
target_link_libraries(my_app PRIVATE libA)

# Visibility controls transitivity:
# PRIVATE: Not propagated to consumers
# PUBLIC: Propagated to consumers (build + usage)
# INTERFACE: Propagated to consumers (usage only)
)" << std::endl;
}

void demonstrate_target_usage() {
    std::cout << "\n=== Target Usage Requirements ===" << std::endl;
    
    std::cout << "\nUsage Requirements are properties that are propagated" << std::endl;
    std::cout << "to targets that link to this target:" << std::endl;
    
    std::cout << R"(
# Set usage requirements
set_target_properties(mylib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    INTERFACE_COMPILE_DEFINITIONS "USING_MYLIB=1"
    INTERFACE_COMPILE_FEATURES "cxx_std_17"
    INTERFACE_LINK_LIBRARIES "Threads::Threads")

# Modern approach using target_* commands
target_include_directories(mylib INTERFACE include/)
target_compile_definitions(mylib INTERFACE USING_MYLIB=1)
target_compile_features(mylib INTERFACE cxx_std_17)
target_link_libraries(mylib INTERFACE Threads::Threads)

# Consumers automatically inherit these requirements
target_link_libraries(my_app PRIVATE mylib)
# my_app now has:
# - include/ in include path
# - USING_MYLIB=1 defined
# - C++17 required
# - Threads::Threads linked
)" << std::endl;
}

// =============================================================================
// PRACTICAL DEMONSTRATION
// =============================================================================

void demonstrate_practical_example() {
    std::cout << "\n=== Practical Target Example ===" << std::endl;
    
    Application app("CMake Targets Demo", "1.0.0");
    app.run();
    
    std::cout << "\nThis demonstrates modular design where:" << std::endl;
    std::cout << "- Application class uses TargetLibrary (static/shared lib)" << std::endl;
    std::cout << "- Application class uses InterfaceLibrary (header-only)" << std::endl;
    std::cout << "- Each component would be a separate CMake target" << std::endl;
    
    std::cout << "\nCorresponding CMakeLists.txt:" << std::endl;
    std::cout << R"(
# Interface library (header-only)
add_library(interface_lib INTERFACE)
target_include_directories(interface_lib INTERFACE include/)
target_compile_features(interface_lib INTERFACE cxx_std_17)

# Static library
add_library(target_lib STATIC target_library.cpp)
target_include_directories(target_lib PUBLIC include/)
target_compile_features(target_lib PUBLIC cxx_std_17)
target_link_libraries(target_lib PRIVATE Threads::Threads)

# Main application
add_executable(targets_demo main.cpp application.cpp)
target_link_libraries(targets_demo PRIVATE 
    target_lib 
    interface_lib)

# Create alias for consistent naming
add_library(MyProject::TargetLib ALIAS target_lib)
add_library(MyProject::InterfaceLib ALIAS interface_lib)
)" << std::endl;
}

int main() {
    std::cout << "CMAKE TARGETS AND TARGET PROPERTIES\n";
    std::cout << "===================================\n";
    
    demonstrate_target_types();
    demonstrate_target_properties();
    demonstrate_target_commands();
    demonstrate_generator_expressions();
    demonstrate_target_dependencies();
    demonstrate_target_usage();
    demonstrate_practical_example();
    
    std::cout << "\nKey Target Concepts:" << std::endl;
    std::cout << "- Targets are the fundamental unit in CMake" << std::endl;
    std::cout << "- Use target_* commands for modern CMake" << std::endl;
    std::cout << "- Visibility (PRIVATE/PUBLIC/INTERFACE) controls propagation" << std::endl;
    std::cout << "- Generator expressions enable build-time conditionals" << std::endl;
    std::cout << "- Usage requirements automatically propagate to consumers" << std::endl;
    std::cout << "- Alias targets provide consistent naming" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- CMake targets encapsulate build requirements and dependencies
- Modern CMake uses target-centric approach vs directory-based
- Visibility keywords control what gets propagated to consumers
- Generator expressions provide powerful conditional logic
- Usage requirements eliminate manual configuration propagation
- Imported targets represent external dependencies
- Alias targets enable consistent naming across projects
- Target properties control every aspect of compilation and linking
*/
