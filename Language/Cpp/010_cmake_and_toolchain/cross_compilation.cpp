/*
 * CMAKE AND TOOLCHAIN - CROSS COMPILATION
 * 
 * This file demonstrates cross-compilation concepts and CMake
 * toolchain configuration for different target platforms.
 * 
 * Shows how to configure CMake for cross-platform builds.
 */

#include <iostream>
#include <string>

// =============================================================================
// PLATFORM DETECTION AND ADAPTATION
// =============================================================================

class PlatformInfo {
public:
    static void display_platform_info() {
        std::cout << "Platform Information:" << std::endl;
        
#ifdef _WIN32
        std::cout << "  Operating System: Windows" << std::endl;
    #ifdef _WIN64
        std::cout << "  Architecture: x64" << std::endl;
    #else
        std::cout << "  Architecture: x86" << std::endl;
    #endif
#elif __linux__
        std::cout << "  Operating System: Linux" << std::endl;
    #ifdef __x86_64__
        std::cout << "  Architecture: x86_64" << std::endl;
    #elif __i386__
        std::cout << "  Architecture: i386" << std::endl;
    #elif __arm__
        std::cout << "  Architecture: ARM" << std::endl;
    #elif __aarch64__
        std::cout << "  Architecture: ARM64" << std::endl;
    #endif
#elif __APPLE__
        std::cout << "  Operating System: macOS" << std::endl;
    #ifdef __x86_64__
        std::cout << "  Architecture: x86_64" << std::endl;
    #elif __arm64__
        std::cout << "  Architecture: ARM64 (Apple Silicon)" << std::endl;
    #endif
#else
        std::cout << "  Operating System: Unknown" << std::endl;
#endif

        // Compiler information
#ifdef __GNUC__
        std::cout << "  Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << std::endl;
#elif _MSC_VER
        std::cout << "  Compiler: MSVC " << _MSC_VER << std::endl;
#elif __clang__
        std::cout << "  Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << std::endl;
#endif

        // C++ standard
        std::cout << "  C++ Standard: " << __cplusplus << std::endl;
    }
    
    static std::string get_platform_specific_message() {
#ifdef _WIN32
        return "Hello from Windows!";
#elif __linux__
        return "Hello from Linux!";
#elif __APPLE__
        return "Hello from macOS!";
#else
        return "Hello from unknown platform!";
#endif
    }
};

// =============================================================================
// CMAKE TOOLCHAIN FILE EXAMPLES
// =============================================================================

void demonstrate_toolchain_files() {
    std::cout << "\n=== CMake Toolchain Files ===" << std::endl;
    
    std::cout << "\nToolchain files specify cross-compilation settings:" << std::endl;
    
    std::cout << "\n1. Linux to Windows (MinGW) Toolchain:" << std::endl;
    std::cout << R"(
# toolchain-mingw.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Where to find libraries and headers
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set executable suffix
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
)" << std::endl;

    std::cout << "\n2. x86_64 to ARM64 Linux Toolchain:" << std::endl;
    std::cout << R"(
# toolchain-arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross compiler
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Where to find libraries and headers
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# Search settings
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler flags
set(CMAKE_C_FLAGS "-march=armv8-a")
set(CMAKE_CXX_FLAGS "-march=armv8-a")
)" << std::endl;

    std::cout << "\n3. Embedded ARM Cortex-M Toolchain:" << std::endl;
    std::cout << R"(
# toolchain-arm-none-eabi.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)

# Processor specific flags
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections -specs=nano.specs")

# No standard libraries
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
)" << std::endl;
}

void demonstrate_cross_compilation_cmake() {
    std::cout << "\n=== Cross-Compilation CMake Configuration ===" << std::endl;
    
    std::cout << "\nCMakeLists.txt for cross-compilation:" << std::endl;
    std::cout << R"(
cmake_minimum_required(VERSION 3.16)
project(CrossCompileExample LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific configurations
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring for Windows")
    # Windows-specific settings
    add_compile_definitions(PLATFORM_WINDOWS)
    if(MINGW)
        # MinGW specific settings
        target_link_libraries(my_app PRIVATE ws2_32)
    endif()
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring for Linux")
    # Linux-specific settings
    add_compile_definitions(PLATFORM_LINUX)
    find_package(Threads REQUIRED)
    target_link_libraries(my_app PRIVATE Threads::Threads)
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring for macOS")
    # macOS-specific settings
    add_compile_definitions(PLATFORM_MACOS)
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    message(STATUS "Configuring for embedded system")
    # Embedded-specific settings
    add_compile_definitions(PLATFORM_EMBEDDED)
    set(CMAKE_EXECUTABLE_SUFFIX ".elf")
endif()

# Architecture-specific settings
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_compile_definitions(ARCH_X64)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    add_compile_definitions(ARCH_ARM64)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    add_compile_definitions(ARCH_ARM32)
endif()

# Create executable
add_executable(my_app main.cpp)

# Platform-specific linking
if(WIN32)
    target_link_libraries(my_app PRIVATE kernel32 user32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(my_app PRIVATE m dl)
elseif(APPLE)
    find_library(FOUNDATION Foundation)
    target_link_libraries(my_app PRIVATE ${FOUNDATION})
endif()
)" << std::endl;

    std::cout << "\nBuild commands for cross-compilation:" << std::endl;
    std::cout << "# Configure with toolchain file" << std::endl;
    std::cout << "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -B build-windows" << std::endl;
    std::cout << "cmake --build build-windows" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "# Multiple target builds" << std::endl;
    std::cout << "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-arm64.cmake -B build-arm64" << std::endl;
    std::cout << "cmake --build build-arm64" << std::endl;
}

void demonstrate_conditional_compilation() {
    std::cout << "\n=== Conditional Compilation Techniques ===" << std::endl;
    
    std::cout << "\n1. Preprocessor Macros:" << std::endl;
    std::cout << R"(
// Platform-specific code
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    void platform_sleep(int ms) {
        Sleep(ms);
    }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
    #include <unistd.h>
    void platform_sleep(int ms) {
        usleep(ms * 1000);
    }
#endif

// Architecture-specific optimizations
#ifdef ARCH_X64
    void optimized_function() {
        // x64 specific optimizations
        __asm__("nop");
    }
#elif defined(ARCH_ARM64)
    void optimized_function() {
        // ARM64 specific optimizations
        __asm__("nop");
    }
#endif
)" << std::endl;

    std::cout << "\n2. CMake Generator Expressions:" << std::endl;
    std::cout << R"(
# Conditional source files
target_sources(my_app PRIVATE
    main.cpp
    $<$<PLATFORM_ID:Windows>:windows_specific.cpp>
    $<$<PLATFORM_ID:Linux>:linux_specific.cpp>
    $<$<PLATFORM_ID:Darwin>:macos_specific.cpp>)

# Conditional libraries
target_link_libraries(my_app PRIVATE
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Linux>:pthread>
    $<$<PLATFORM_ID:Darwin>:"-framework Foundation">)

# Conditional compile definitions
target_compile_definitions(my_app PRIVATE
    $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Linux>:PLATFORM_LINUX>
    $<$<PLATFORM_ID:Darwin>:PLATFORM_MACOS>)
)" << std::endl;

    std::cout << "\n3. CMake Platform Variables:" << std::endl;
    std::cout << R"(
# Check platform in CMake
if(WIN32)
    message(STATUS "Building for Windows")
    set(PLATFORM_SOURCES windows_impl.cpp)
elseif(UNIX)
    if(APPLE)
        message(STATUS "Building for macOS")
        set(PLATFORM_SOURCES macos_impl.cpp)
    else()
        message(STATUS "Building for Linux")
        set(PLATFORM_SOURCES linux_impl.cpp)
    endif()
endif()

target_sources(my_app PRIVATE ${PLATFORM_SOURCES})
)" << std::endl;
}

void demonstrate_cross_compilation_challenges() {
    std::cout << "\n=== Cross-Compilation Challenges ===" << std::endl;
    
    std::cout << "\n1. Library Dependencies:" << std::endl;
    std::cout << "   • Target libraries must be available for target platform" << std::endl;
    std::cout << "   • Package managers: vcpkg, Conan, or manual builds" << std::endl;
    std::cout << "   • Static linking reduces runtime dependencies" << std::endl;
    
    std::cout << "\n2. System Differences:" << std::endl;
    std::cout << "   • File paths (/ vs \\)" << std::endl;
    std::cout << "   • Line endings (LF vs CRLF)" << std::endl;
    std::cout << "   • Case sensitivity" << std::endl;
    std::cout << "   • Executable extensions (.exe)" << std::endl;
    
    std::cout << "\n3. Architecture Differences:" << std::endl;
    std::cout << "   • Endianness (big-endian vs little-endian)" << std::endl;
    std::cout << "   • Pointer sizes (32-bit vs 64-bit)" << std::endl;
    std::cout << "   • Alignment requirements" << std::endl;
    std::cout << "   • Available instruction sets" << std::endl;
    
    std::cout << "\n4. Testing Challenges:" << std::endl;
    std::cout << "   • Cannot run target binaries on host" << std::endl;
    std::cout << "   • Need emulation or target hardware" << std::endl;
    std::cout << "   • Remote testing setups" << std::endl;
    std::cout << "   • Docker containers for testing" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== Cross-Compilation Best Practices ===" << std::endl;
    
    std::cout << "\n1. Toolchain Management:" << std::endl;
    std::cout << "   • Use consistent toolchain versions" << std::endl;
    std::cout << "   • Document toolchain setup procedures" << std::endl;
    std::cout << "   • Automate toolchain installation" << std::endl;
    std::cout << "   • Version control toolchain files" << std::endl;
    
    std::cout << "\n2. Build Organization:" << std::endl;
    std::cout << "   • Separate build directories per target" << std::endl;
    std::cout << "   • Use descriptive build directory names" << std::endl;
    std::cout << "   • Parallel builds for multiple targets" << std::endl;
    std::cout << "   • CI/CD integration for all targets" << std::endl;
    
    std::cout << "\n3. Code Organization:" << std::endl;
    std::cout << "   • Abstract platform-specific code" << std::endl;
    std::cout << "   • Use interfaces for platform differences" << std::endl;
    std::cout << "   • Minimize conditional compilation" << std::endl;
    std::cout << "   • Test on all target platforms regularly" << std::endl;
    
    std::cout << "\n4. Dependency Management:" << std::endl;
    std::cout << "   • Use package managers when possible" << std::endl;
    std::cout << "   • Document external dependencies" << std::endl;
    std::cout << "   • Consider static linking for deployment" << std::endl;
    std::cout << "   • Maintain dependency compatibility matrix" << std::endl;
}

void demonstrate_docker_cross_compilation() {
    std::cout << "\n=== Docker-based Cross-Compilation ===" << std::endl;
    
    std::cout << "\nDocker provides isolated cross-compilation environments:" << std::endl;
    
    std::cout << "\nDockerfile for MinGW cross-compilation:" << std::endl;
    std::cout << R"(
FROM ubuntu:20.04

# Install cross-compilation tools
RUN apt-get update && apt-get install -y \
    gcc-mingw-w64-x86-64 \
    g++-mingw-w64-x86-64 \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . .

# Configure and build
RUN cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -B build
RUN cmake --build build

# Output will be Windows executable
)" << std::endl;

    std::cout << "\nDocker commands:" << std::endl;
    std::cout << "docker build -t cross-compiler ." << std::endl;
    std::cout << "docker run --rm -v $(pwd):/workspace cross-compiler" << std::endl;
    
    std::cout << "\nBenefits of Docker approach:" << std::endl;
    std::cout << "• Consistent build environment" << std::endl;
    std::cout << "• Easy CI/CD integration" << std::endl;
    std::cout << "• No host system pollution" << std::endl;
    std::cout << "• Reproducible builds" << std::endl;
}

// =============================================================================
// PRACTICAL DEMONSTRATION
// =============================================================================

void demonstrate_practical_example() {
    std::cout << "\n=== Practical Cross-Compilation Example ===" << std::endl;
    
    PlatformInfo::display_platform_info();
    std::cout << "\nPlatform message: " << PlatformInfo::get_platform_specific_message() << std::endl;
    
    std::cout << "\nThis application demonstrates:" << std::endl;
    std::cout << "• Runtime platform detection" << std::endl;
    std::cout << "• Conditional compilation based on platform" << std::endl;
    std::cout << "• Cross-platform compatible code structure" << std::endl;
    
    std::cout << "\nTo cross-compile this application:" << std::endl;
    std::cout << "1. Set up target toolchain" << std::endl;
    std::cout << "2. Create appropriate toolchain file" << std::endl;
    std::cout << "3. Configure with toolchain: cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake" << std::endl;
    std::cout << "4. Build: cmake --build build" << std::endl;
    std::cout << "5. Test on target platform" << std::endl;
}

int main() {
    std::cout << "CMAKE CROSS-COMPILATION TUTORIAL\n";
    std::cout << "================================\n";
    
    demonstrate_toolchain_files();
    demonstrate_cross_compilation_cmake();
    demonstrate_conditional_compilation();
    demonstrate_cross_compilation_challenges();
    demonstrate_best_practices();
    demonstrate_docker_cross_compilation();
    demonstrate_practical_example();
    
    std::cout << "\nKey Cross-Compilation Concepts:" << std::endl;
    std::cout << "• Toolchain files specify cross-compilation settings" << std::endl;
    std::cout << "• Conditional compilation handles platform differences" << std::endl;
    std::cout << "• Separate build directories for each target" << std::endl;
    std::cout << "• Testing requires target platform or emulation" << std::endl;
    std::cout << "• Docker provides consistent build environments" << std::endl;
    std::cout << "• Abstract platform-specific code for maintainability" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Cross-compilation enables building for different target platforms
- Toolchain files encapsulate all cross-compilation settings
- CMake provides excellent cross-compilation support
- Platform abstraction reduces conditional compilation complexity
- Docker containers provide reproducible cross-compilation environments
- Testing cross-compiled binaries requires target hardware or emulation
- Dependency management is more complex with cross-compilation
- Automation and CI/CD are crucial for multi-platform projects
*/
