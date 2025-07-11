/*
 * CMAKE AND TOOLCHAIN - MODULAR PROJECT STRUCTURE
 * 
 * This file demonstrates CMake concepts through C++ code examples
 * and shows how to structure modular C++ projects with CMake.
 * 
 * Compilation: See CMakeLists.txt examples below
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// =============================================================================
// EXAMPLE PROJECT STRUCTURE DEMONSTRATION
// =============================================================================

namespace ModularProject {

// Library module example
class MathLibrary {
public:
    static double add(double a, double b) { return a + b; }
    static double multiply(double a, double b) { return a * b; }
    static double power(double base, int exp) {
        double result = 1.0;
        for (int i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }
};

// Utility module example
class StringUtils {
public:
    static std::string to_upper(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            c = std::toupper(c);
        }
        return result;
    }
    
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : str) {
            if (c == delimiter) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

// Configuration module example
class Config {
private:
    static inline std::string app_name_ = "ModularApp";
    static inline std::string version_ = "1.0.0";
    
public:
    static const std::string& get_app_name() { return app_name_; }
    static const std::string& get_version() { return version_; }
    static void set_app_name(const std::string& name) { app_name_ = name; }
    static void set_version(const std::string& version) { version_ = version; }
};

} // namespace ModularProject

// =============================================================================
// CMAKE PROJECT STRUCTURE EXAMPLES
// =============================================================================

void demonstrate_project_structure() {
    std::cout << "\n=== CMake Modular Project Structure ===" << std::endl;
    
    std::cout << "\nRecommended project structure:" << std::endl;
    std::cout << "project_root/" << std::endl;
    std::cout << "├── CMakeLists.txt              # Root CMake file" << std::endl;
    std::cout << "├── src/" << std::endl;
    std::cout << "│   ├── CMakeLists.txt          # Source CMake file" << std::endl;
    std::cout << "│   ├── main.cpp                # Application entry point" << std::endl;
    std::cout << "│   └── app/" << std::endl;
    std::cout << "│       ├── CMakeLists.txt      # App module CMake" << std::endl;
    std::cout << "│       ├── application.cpp" << std::endl;
    std::cout << "│       └── application.h" << std::endl;
    std::cout << "├── libs/" << std::endl;
    std::cout << "│   ├── math/" << std::endl;
    std::cout << "│   │   ├── CMakeLists.txt      # Math library CMake" << std::endl;
    std::cout << "│   │   ├── include/math/" << std::endl;
    std::cout << "│   │   │   └── math.h" << std::endl;
    std::cout << "│   │   └── src/" << std::endl;
    std::cout << "│   │       └── math.cpp" << std::endl;
    std::cout << "│   └── utils/" << std::endl;
    std::cout << "│       ├── CMakeLists.txt      # Utils library CMake" << std::endl;
    std::cout << "│       ├── include/utils/" << std::endl;
    std::cout << "│       │   └── string_utils.h" << std::endl;
    std::cout << "│       └── src/" << std::endl;
    std::cout << "│           └── string_utils.cpp" << std::endl;
    std::cout << "├── tests/" << std::endl;
    std::cout << "│   ├── CMakeLists.txt          # Tests CMake file" << std::endl;
    std::cout << "│   ├── test_math.cpp" << std::endl;
    std::cout << "│   └── test_utils.cpp" << std::endl;
    std::cout << "├── external/                   # Third-party dependencies" << std::endl;
    std::cout << "├── build/                      # Build directory (gitignored)" << std::endl;
    std::cout << "└── docs/                       # Documentation" << std::endl;
}

void demonstrate_cmake_examples() {
    std::cout << "\n=== CMake File Examples ===" << std::endl;
    
    std::cout << "\nRoot CMakeLists.txt:" << std::endl;
    std::cout << R"(
cmake_minimum_required(VERSION 3.16)

project(ModularProject 
    VERSION 1.0.0
    DESCRIPTION "A modular C++ project example"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build test programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/libs/math/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/utils/include)

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install configuration
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
)" << std::endl;

    std::cout << "\nLibrary CMakeLists.txt (libs/math/CMakeLists.txt):" << std::endl;
    std::cout << R"(
# Math library
add_library(math_lib
    src/math.cpp
    include/math/math.h)

target_include_directories(math_lib
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(math_lib PUBLIC cxx_std_17)

# Alias for consistent naming
add_library(ModularProject::math ALIAS math_lib)

# Install rules
install(TARGETS math_lib
    EXPORT MathLibTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)
)" << std::endl;

    std::cout << "\nApplication CMakeLists.txt (src/CMakeLists.txt):" << std::endl;
    std::cout << R"(
# Main executable
add_executable(${PROJECT_NAME}
    main.cpp
    app/application.cpp
    app/application.h)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ModularProject::math
        ModularProject::utils)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app)

# Set executable properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "modular_app"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
)" << std::endl;
}

void demonstrate_cmake_variables() {
    std::cout << "\n=== Important CMake Variables ===" << std::endl;
    
    std::cout << "\nProject Variables:" << std::endl;
    std::cout << "PROJECT_NAME          - Name of the project" << std::endl;
    std::cout << "PROJECT_VERSION       - Version of the project" << std::endl;
    std::cout << "PROJECT_SOURCE_DIR    - Source directory of the project" << std::endl;
    std::cout << "PROJECT_BINARY_DIR    - Binary directory of the project" << std::endl;
    
    std::cout << "\nCMake Variables:" << std::endl;
    std::cout << "CMAKE_SOURCE_DIR      - Top-level source directory" << std::endl;
    std::cout << "CMAKE_BINARY_DIR      - Top-level binary directory" << std::endl;
    std::cout << "CMAKE_CURRENT_SOURCE_DIR - Current source directory" << std::endl;
    std::cout << "CMAKE_CURRENT_BINARY_DIR - Current binary directory" << std::endl;
    
    std::cout << "\nCompiler Variables:" << std::endl;
    std::cout << "CMAKE_CXX_COMPILER    - C++ compiler path" << std::endl;
    std::cout << "CMAKE_CXX_STANDARD    - C++ standard version" << std::endl;
    std::cout << "CMAKE_BUILD_TYPE      - Build configuration" << std::endl;
    std::cout << "CMAKE_CXX_FLAGS       - C++ compiler flags" << std::endl;
    
    std::cout << "\nInstall Variables:" << std::endl;
    std::cout << "CMAKE_INSTALL_PREFIX  - Installation prefix" << std::endl;
    std::cout << "CMAKE_INSTALL_BINDIR  - Binary installation directory" << std::endl;
    std::cout << "CMAKE_INSTALL_LIBDIR  - Library installation directory" << std::endl;
    std::cout << "CMAKE_INSTALL_INCLUDEDIR - Header installation directory" << std::endl;
}

void demonstrate_cmake_commands() {
    std::cout << "\n=== Essential CMake Commands ===" << std::endl;
    
    std::cout << "\nProject Definition:" << std::endl;
    std::cout << "cmake_minimum_required(VERSION 3.16)" << std::endl;
    std::cout << "project(MyProject VERSION 1.0.0 LANGUAGES CXX)" << std::endl;
    
    std::cout << "\nTarget Creation:" << std::endl;
    std::cout << "add_executable(app main.cpp)           # Create executable" << std::endl;
    std::cout << "add_library(mylib lib.cpp)             # Create library" << std::endl;
    std::cout << "add_library(mylib SHARED lib.cpp)      # Create shared library" << std::endl;
    std::cout << "add_library(mylib STATIC lib.cpp)      # Create static library" << std::endl;
    
    std::cout << "\nTarget Configuration:" << std::endl;
    std::cout << "target_link_libraries(app mylib)       # Link libraries" << std::endl;
    std::cout << "target_include_directories(app PRIVATE include/) # Add includes" << std::endl;
    std::cout << "target_compile_definitions(app PRIVATE MY_DEFINE=1) # Add defines" << std::endl;
    std::cout << "target_compile_features(app PUBLIC cxx_std_17) # Require C++17" << std::endl;
    
    std::cout << "\nFind Packages:" << std::endl;
    std::cout << "find_package(Threads REQUIRED)         # Find system threads" << std::endl;
    std::cout << "find_package(Boost REQUIRED COMPONENTS system filesystem)" << std::endl;
    
    std::cout << "\nInstallation:" << std::endl;
    std::cout << "install(TARGETS app DESTINATION bin)   # Install executable" << std::endl;
    std::cout << "install(FILES header.h DESTINATION include/) # Install headers" << std::endl;
}

void demonstrate_build_commands() {
    std::cout << "\n=== CMake Build Commands ===" << std::endl;
    
    std::cout << "\nBasic Build Process:" << std::endl;
    std::cout << "mkdir build && cd build                 # Create build directory" << std::endl;
    std::cout << "cmake ..                                # Configure project" << std::endl;
    std::cout << "cmake --build .                         # Build project" << std::endl;
    std::cout << "cmake --install .                       # Install project" << std::endl;
    
    std::cout << "\nBuild Type Configuration:" << std::endl;
    std::cout << "cmake -DCMAKE_BUILD_TYPE=Debug ..       # Debug build" << std::endl;
    std::cout << "cmake -DCMAKE_BUILD_TYPE=Release ..     # Release build" << std::endl;
    std::cout << "cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. # Release with debug info" << std::endl;
    
    std::cout << "\nGenerator Selection:" << std::endl;
    std::cout << "cmake -G \"Unix Makefiles\" ..           # Use Make" << std::endl;
    std::cout << "cmake -G \"Ninja\" ..                    # Use Ninja" << std::endl;
    std::cout << "cmake -G \"Visual Studio 16 2019\" ..   # Use Visual Studio" << std::endl;
    
    std::cout << "\nParallel Building:" << std::endl;
    std::cout << "cmake --build . --parallel 4           # Build with 4 threads" << std::endl;
    std::cout << "cmake --build . -j 4                    # Alternative syntax" << std::endl;
    
    std::cout << "\nCustom Options:" << std::endl;
    std::cout << "cmake -DBUILD_TESTS=ON ..               # Enable tests" << std::endl;
    std::cout << "cmake -DBUILD_SHARED_LIBS=ON ..         # Build shared libraries" << std::endl;
    std::cout << "cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. # Set install prefix" << std::endl;
}

// =============================================================================
// PRACTICAL EXAMPLE USAGE
// =============================================================================

void demonstrate_modular_usage() {
    std::cout << "\n=== Modular Project Usage Example ===" << std::endl;
    
    using namespace ModularProject;
    
    // Configure the application
    Config::set_app_name("CMake Demo App");
    Config::set_version("2.1.0");
    
    std::cout << "Application: " << Config::get_app_name() 
              << " v" << Config::get_version() << std::endl;
    
    // Use math library
    double a = 5.0, b = 3.0;
    std::cout << "Math operations:" << std::endl;
    std::cout << "  " << a << " + " << b << " = " << MathLibrary::add(a, b) << std::endl;
    std::cout << "  " << a << " * " << b << " = " << MathLibrary::multiply(a, b) << std::endl;
    std::cout << "  " << a << "^3 = " << MathLibrary::power(a, 3) << std::endl;
    
    // Use string utilities
    std::string text = "hello,world,cmake";
    std::cout << "\nString operations:" << std::endl;
    std::cout << "  Original: " << text << std::endl;
    std::cout << "  Uppercase: " << StringUtils::to_upper(text) << std::endl;
    
    auto tokens = StringUtils::split(text, ',');
    std::cout << "  Split tokens: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== CMake Best Practices ===" << std::endl;
    
    std::cout << "\n1. Project Organization:" << std::endl;
    std::cout << "   - Use clear directory structure" << std::endl;
    std::cout << "   - Separate source, headers, and tests" << std::endl;
    std::cout << "   - Keep CMakeLists.txt files close to sources" << std::endl;
    
    std::cout << "\n2. Target-based Approach:" << std::endl;
    std::cout << "   - Use target_* commands instead of global settings" << std::endl;
    std::cout << "   - Prefer PRIVATE/PUBLIC/INTERFACE visibility" << std::endl;
    std::cout << "   - Create ALIAS targets for consistency" << std::endl;
    
    std::cout << "\n3. Modern CMake (3.16+):" << std::endl;
    std::cout << "   - Use target_compile_features() for C++ standard" << std::endl;
    std::cout << "   - Avoid CMAKE_CXX_FLAGS for target-specific flags" << std::endl;
    std::cout << "   - Use generator expressions for conditional logic" << std::endl;
    
    std::cout << "\n4. Package Management:" << std::endl;
    std::cout << "   - Use find_package() for system dependencies" << std::endl;
    std::cout << "   - Consider FetchContent for external projects" << std::endl;
    std::cout << "   - Create Config.cmake files for your libraries" << std::endl;
    
    std::cout << "\n5. Build Configuration:" << std::endl;
    std::cout << "   - Always use out-of-source builds" << std::endl;
    std::cout << "   - Set reasonable default build type" << std::endl;
    std::cout << "   - Use options for configurable features" << std::endl;
}

int main() {
    std::cout << "CMAKE MODULAR PROJECT STRUCTURE\n";
    std::cout << "===============================\n";
    
    demonstrate_project_structure();
    demonstrate_cmake_examples();
    demonstrate_cmake_variables();
    demonstrate_cmake_commands();
    demonstrate_build_commands();
    demonstrate_modular_usage();
    demonstrate_best_practices();
    
    std::cout << "\nNext Steps:" << std::endl;
    std::cout << "1. Create the recommended directory structure" << std::endl;
    std::cout << "2. Write CMakeLists.txt files for each component" << std::endl;
    std::cout << "3. Build with: mkdir build && cd build && cmake .. && cmake --build ." << std::endl;
    std::cout << "4. Test with: ctest or cmake --build . --target test" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- CMake enables modular, maintainable C++ projects
- Target-based approach is modern CMake best practice
- Clear directory structure improves project organization
- Generator expressions provide build-time flexibility
- Out-of-source builds keep source tree clean
- Package management simplifies dependency handling
- Proper visibility (PRIVATE/PUBLIC) controls interface
- Modern CMake (3.16+) offers powerful features
*/
