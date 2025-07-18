/*
 * CMAKE AND TOOLCHAIN - MAKEFILE VS CMAKE COMPARISON
 * 
 * This file demonstrates the differences between traditional Makefiles
 * and modern CMake through practical examples.
 * 
 * Shows equivalent build configurations in both systems.
 */

#include <iostream>
#include <string>
#include <vector>

// =============================================================================
// SAMPLE PROJECT FOR COMPARISON
// =============================================================================

// Simple math library for demonstration
class MathLibrary {
public:
    static int add(int a, int b) { return a + b; }
    static int subtract(int a, int b) { return a - b; }
    static int multiply(int a, int b) { return a * b; }
    static double divide(double a, double b) { return b != 0 ? a / b : 0; }
    
    static void print_info() {
        std::cout << "MathLibrary - Simple arithmetic operations" << std::endl;
    }
};

// Utility functions
class Utils {
public:
    static std::string format_result(const std::string& operation, double result) {
        return operation + " = " + std::to_string(result);
    }
    
    static void demonstrate_operations() {
        std::cout << "Demonstrating math operations:" << std::endl;
        std::cout << Utils::format_result("10 + 5", MathLibrary::add(10, 5)) << std::endl;
        std::cout << Utils::format_result("10 - 5", MathLibrary::subtract(10, 5)) << std::endl;
        std::cout << Utils::format_result("10 * 5", MathLibrary::multiply(10, 5)) << std::endl;
        std::cout << Utils::format_result("10 / 5", MathLibrary::divide(10, 5)) << std::endl;
    }
};

// =============================================================================
// MAKEFILE EXAMPLES
// =============================================================================

void demonstrate_makefile_approach() {
    std::cout << "\n=== Traditional Makefile Approach ===" << std::endl;
    
    std::cout << "\nBasic Makefile example:" << std::endl;
    std::cout << R"(
# Basic Makefile
CC = g++
CFLAGS = -Wall -Wextra -std=c++17
TARGET = my_app
SOURCES = main.cpp math.cpp utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
)" << std::endl;

    std::cout << "\nAdvanced Makefile with libraries:" << std::endl;
    std::cout << R"(
# Advanced Makefile
CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -O2
LDFLAGS = -pthread
INCLUDES = -Iinclude -I/usr/local/include
LIBDIRS = -Llib -L/usr/local/lib
LIBS = -lmath -lpthread

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = bin
LIBDIR = lib

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Targets
TARGET = $(BINDIR)/my_app
LIBRARY = $(LIBDIR)/libmath.a

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(OBJDIR) $(BINDIR) $(LIBDIR)

# Build executable
$(TARGET): $(OBJECTS) $(LIBRARY)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBDIRS) $(LIBS) -o $@

# Build static library
$(LIBRARY): $(OBJDIR)/math.o
	ar rcs $@ $^

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Install target
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	cp $(LIBRARY) /usr/local/lib/

# Clean
clean:
	rm -rf $(OBJDIR) $(BINDIR) $(LIBDIR)

# Dependencies (would typically be auto-generated)
$(OBJDIR)/main.o: $(SRCDIR)/main.cpp include/math.h include/utils.h
$(OBJDIR)/math.o: $(SRCDIR)/math.cpp include/math.h
$(OBJDIR)/utils.o: $(SRCDIR)/utils.cpp include/utils.h

.PHONY: all clean install directories
)" << std::endl;

    std::cout << "\nMakefile characteristics:" << std::endl;
    std::cout << "✓ Direct control over build process" << std::endl;
    std::cout << "✓ Fast execution (minimal overhead)" << std::endl;
    std::cout << "✓ Universal availability on Unix systems" << std::endl;
    std::cout << "✗ Platform-specific (different make variants)" << std::endl;
    std::cout << "✗ Manual dependency management" << std::endl;
    std::cout << "✗ Complex syntax for advanced features" << std::endl;
    std::cout << "✗ No built-in package management" << std::endl;
}

// =============================================================================
// CMAKE EXAMPLES
// =============================================================================

void demonstrate_cmake_approach() {
    std::cout << "\n=== Modern CMake Approach ===" << std::endl;
    
    std::cout << "\nBasic CMakeLists.txt example:" << std::endl;
    std::cout << R"(
# Basic CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(my_app
    main.cpp
    math.cpp
    utils.cpp)

# Compiler flags
target_compile_options(my_app PRIVATE
    -Wall -Wextra -O2)

# Find and link threads
find_package(Threads REQUIRED)
target_link_libraries(my_app PRIVATE Threads::Threads)
)" << std::endl;

    std::cout << "\nAdvanced CMakeLists.txt with libraries:" << std::endl;
    std::cout << R"(
# Advanced CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0.0 LANGUAGES CXX)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build test programs" ON)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(include)

# Math library
add_library(math_lib
    src/math.cpp
    include/math.h)

target_include_directories(math_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# Utils library
add_library(utils_lib
    src/utils.cpp
    include/utils.h)

target_include_directories(utils_lib PUBLIC include)
target_link_libraries(utils_lib PUBLIC math_lib)

# Main executable
add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE utils_lib)

# Find packages
find_package(Threads REQUIRED)
target_link_libraries(my_app PRIVATE Threads::Threads)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install rules
include(GNUInstallDirs)
install(TARGETS my_app math_lib utils_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
)" << std::endl;

    std::cout << "\nCMake characteristics:" << std::endl;
    std::cout << "✓ Cross-platform (generates native build files)" << std::endl;
    std::cout << "✓ Automatic dependency detection" << std::endl;
    std::cout << "✓ Built-in package management" << std::endl;
    std::cout << "✓ Modern target-based approach" << std::endl;
    std::cout << "✓ Extensive toolchain support" << std::endl;
    std::cout << "✗ Learning curve for advanced features" << std::endl;
    std::cout << "✗ Additional dependency (cmake tool)" << std::endl;
    std::cout << "✗ Generated files can be complex" << std::endl;
}

// =============================================================================
// DETAILED COMPARISON
// =============================================================================

void demonstrate_detailed_comparison() {
    std::cout << "\n=== Detailed Comparison ===" << std::endl;
    
    std::cout << "\n1. SYNTAX COMPARISON:" << std::endl;
    std::cout << "\nMakefile (imperative):" << std::endl;
    std::cout << "target: dependencies" << std::endl;
    std::cout << "	command" << std::endl;
    std::cout << "	another_command" << std::endl;
    
    std::cout << "\nCMake (declarative):" << std::endl;
    std::cout << "add_executable(target sources...)" << std::endl;
    std::cout << "target_link_libraries(target libs...)" << std::endl;
    
    std::cout << "\n2. DEPENDENCY MANAGEMENT:" << std::endl;
    
    std::cout << "\nMakefile:" << std::endl;
    std::cout << R"(
# Manual dependency specification
main.o: main.cpp math.h utils.h
	$(CC) $(CFLAGS) -c main.cpp

# Or auto-generated dependencies
-include $(OBJECTS:.o=.d)
%.d: %.cpp
	$(CC) -MM $< > $@
)" << std::endl;

    std::cout << "\nCMake:" << std::endl;
    std::cout << R"(
# Automatic dependency scanning
add_executable(app main.cpp)
# CMake automatically tracks header dependencies
)" << std::endl;

    std::cout << "\n3. CROSS-PLATFORM SUPPORT:" << std::endl;
    
    std::cout << "\nMakefile:" << std::endl;
    std::cout << R"(
# Platform-specific sections
ifeq ($(OS),Windows_NT)
    TARGET = app.exe
    RM = del
else
    TARGET = app
    RM = rm -f
endif
)" << std::endl;

    std::cout << "\nCMake:" << std::endl;
    std::cout << R"(
# Automatic platform handling
add_executable(app main.cpp)
# CMake handles .exe extension, path separators, etc.
)" << std::endl;

    std::cout << "\n4. LIBRARY HANDLING:" << std::endl;
    
    std::cout << "\nMakefile:" << std::endl;
    std::cout << R"(
# Manual library specification
LIBS = -lpthread -lm -ldl
LDFLAGS = -L/usr/local/lib

app: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
)" << std::endl;

    std::cout << "\nCMake:" << std::endl;
    std::cout << R"(
# Package-based library handling
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Threads::Threads)
)" << std::endl;
}

void demonstrate_build_comparison() {
    std::cout << "\n=== Build Process Comparison ===" << std::endl;
    
    std::cout << "\nMakefile build process:" << std::endl;
    std::cout << "1. Write Makefile manually" << std::endl;
    std::cout << "2. Run: make" << std::endl;
    std::cout << "3. Run: make install (if defined)" << std::endl;
    std::cout << "4. Run: make clean" << std::endl;
    
    std::cout << "\nCMake build process:" << std::endl;
    std::cout << "1. Write CMakeLists.txt" << std::endl;
    std::cout << "2. Configure: cmake -B build" << std::endl;
    std::cout << "3. Build: cmake --build build" << std::endl;
    std::cout << "4. Install: cmake --install build" << std::endl;
    std::cout << "5. Clean: cmake --build build --target clean" << std::endl;
    
    std::cout << "\nGenerator support:" << std::endl;
    std::cout << "Makefile: Unix Makefiles only" << std::endl;
    std::cout << "CMake: Make, Ninja, Visual Studio, Xcode, etc." << std::endl;
    
    std::cout << "\nParallel builds:" << std::endl;
    std::cout << "Makefile: make -j4" << std::endl;
    std::cout << "CMake: cmake --build build --parallel 4" << std::endl;
}

void demonstrate_when_to_use() {
    std::cout << "\n=== When to Use Each ===" << std::endl;
    
    std::cout << "\nUse Makefiles when:" << std::endl;
    std::cout << "• Simple, single-platform projects" << std::endl;
    std::cout << "• Maximum control over build process" << std::endl;
    std::cout << "• Minimal dependencies (no cmake)" << std::endl;
    std::cout << "• Legacy system compatibility" << std::endl;
    std::cout << "• Custom build workflows" << std::endl;
    std::cout << "• Embedded systems with constraints" << std::endl;
    
    std::cout << "\nUse CMake when:" << std::endl;
    std::cout << "• Cross-platform development" << std::endl;
    std::cout << "• Complex dependency management" << std::endl;
    std::cout << "• Modern C++ projects" << std::endl;
    std::cout << "• Team development" << std::endl;
    std::cout << "• Package distribution" << std::endl;
    std::cout << "• Integration with IDEs" << std::endl;
    std::cout << "• Third-party library usage" << std::endl;
    
    std::cout << "\nHybrid approach:" << std::endl;
    std::cout << "• Use CMake for configuration" << std::endl;
    std::cout << "• Generate Makefiles for building" << std::endl;
    std::cout << "• Best of both worlds" << std::endl;
}

void demonstrate_migration_tips() {
    std::cout << "\n=== Migration from Makefile to CMake ===" << std::endl;
    
    std::cout << "\nStep-by-step migration:" << std::endl;
    std::cout << "1. Analyze existing Makefile structure" << std::endl;
    std::cout << "2. Identify targets and dependencies" << std::endl;
    std::cout << "3. Create basic CMakeLists.txt" << std::endl;
    std::cout << "4. Convert compiler flags to target properties" << std::endl;
    std::cout << "5. Replace manual library linking with find_package" << std::endl;
    std::cout << "6. Add install rules" << std::endl;
    std::cout << "7. Test on multiple platforms" << std::endl;
    
    std::cout << "\nCommon conversion patterns:" << std::endl;
    std::cout << R"(
Makefile:                   CMake:
--------                    ------
CC = g++                    # Automatic compiler detection
CFLAGS = -Wall -O2          target_compile_options(target PRIVATE -Wall -O2)
INCLUDES = -Iinclude        target_include_directories(target PRIVATE include)
LIBS = -lpthread            find_package(Threads); target_link_libraries(target Threads::Threads)
$(CC) $(CFLAGS) -c $<       # Automatic compilation rules
ar rcs lib.a $(OBJECTS)     add_library(lib STATIC sources...)
)" << std::endl;
}

// =============================================================================
// PRACTICAL DEMONSTRATION
// =============================================================================

void demonstrate_practical_usage() {
    std::cout << "\n=== Practical Usage Example ===" << std::endl;
    
    MathLibrary::print_info();
    Utils::demonstrate_operations();
    
    std::cout << "\nThis code could be built with either:" << std::endl;
    std::cout << "1. Traditional Makefile (manual dependency management)" << std::endl;
    std::cout << "2. Modern CMake (automatic dependency detection)" << std::endl;
    
    std::cout << "\nBoth approaches produce the same executable," << std::endl;
    std::cout << "but CMake provides better maintainability and portability." << std::endl;
}

int main() {
    std::cout << "MAKEFILE VS CMAKE COMPARISON\n";
    std::cout << "============================\n";
    
    demonstrate_makefile_approach();
    demonstrate_cmake_approach();
    demonstrate_detailed_comparison();
    demonstrate_build_comparison();
    demonstrate_when_to_use();
    demonstrate_migration_tips();
    demonstrate_practical_usage();
    
    std::cout << "\nSummary:" << std::endl;
    std::cout << "• Makefiles: Simple, direct, platform-specific" << std::endl;
    std::cout << "• CMake: Complex, abstracted, cross-platform" << std::endl;
    std::cout << "• Choose based on project requirements" << std::endl;
    std::cout << "• CMake is recommended for modern C++ projects" << std::endl;
    std::cout << "• Both tools have their place in the ecosystem" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Makefiles provide direct control but require manual maintenance
- CMake abstracts complexity but adds learning curve
- Cross-platform support is CMake's biggest advantage
- Dependency management is much easier with CMake
- Both tools can coexist (CMake can generate Makefiles)
- Choose based on project complexity and requirements
- Modern C++ projects benefit more from CMake
- Migration from Make to CMake is straightforward
*/
