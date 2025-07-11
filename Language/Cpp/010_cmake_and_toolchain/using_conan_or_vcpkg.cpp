/*
 * CMAKE AND TOOLCHAIN - USING CONAN AND VCPKG PACKAGE MANAGERS
 * 
 * This file demonstrates how to use modern C++ package managers
 * (Conan and vcpkg) with CMake for dependency management.
 * 
 * Shows practical examples of package integration.
 */

#include <iostream>
#include <string>
#include <vector>

// Simulated third-party library usage examples
// In real projects, these would be actual external dependencies

// Example: JSON library usage (like nlohmann/json)
namespace json_example {
    class JsonLibrary {
    public:
        static std::string serialize_data(const std::vector<std::string>& data) {
            std::string json = "[";
            for (size_t i = 0; i < data.size(); ++i) {
                json += "\"" + data[i] + "\"";
                if (i < data.size() - 1) json += ",";
            }
            json += "]";
            return json;
        }
        
        static void demonstrate_usage() {
            std::vector<std::string> data = {"item1", "item2", "item3"};
            std::cout << "JSON output: " << serialize_data(data) << std::endl;
        }
    };
}

// Example: HTTP client library usage (like cpprestsdk or curl)
namespace http_example {
    class HttpClient {
    public:
        static std::string get(const std::string& url) {
            return "HTTP GET response from: " + url;
        }
        
        static void demonstrate_usage() {
            std::string response = get("https://api.example.com/data");
            std::cout << "HTTP Response: " << response << std::endl;
        }
    };
}

// Example: Logging library usage (like spdlog)
namespace logging_example {
    class Logger {
    public:
        static void info(const std::string& message) {
            std::cout << "[INFO] " << message << std::endl;
        }
        
        static void error(const std::string& message) {
            std::cout << "[ERROR] " << message << std::endl;
        }
        
        static void demonstrate_usage() {
            info("Application started");
            error("This is an error message");
        }
    };
}

// =============================================================================
// CONAN PACKAGE MANAGER
// =============================================================================

void demonstrate_conan_basics() {
    std::cout << "\n=== Conan Package Manager ===" << std::endl;
    
    std::cout << "\nConan is a decentralized C++ package manager:" << std::endl;
    std::cout << "• Binary packages for faster builds" << std::endl;
    std::cout << "• Cross-platform support" << std::endl;
    std::cout << "• Dependency version management" << std::endl;
    std::cout << "• Custom package repositories" << std::endl;
    
    std::cout << "\n1. Installation:" << std::endl;
    std::cout << "pip install conan" << std::endl;
    std::cout << "conan profile detect --force  # Create default profile" << std::endl;
    
    std::cout << "\n2. conanfile.txt example:" << std::endl;
    std::cout << R"(
[requires]
nlohmann_json/3.11.2
spdlog/1.12.0
cpprestsdk/2.10.18
boost/1.82.0
openssl/3.1.1

[generators]
CMakeDeps
CMakeToolchain

[options]
boost:shared=False
openssl:shared=False

[imports]
bin, *.dll -> ./bin
lib, *.dylib* -> ./lib
)" << std::endl;

    std::cout << "\n3. CMakeLists.txt with Conan:" << std::endl;
    std::cout << R"(
cmake_minimum_required(VERSION 3.16)
project(ConanExample)

# Find packages installed by Conan
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)

# Create executable
add_executable(my_app main.cpp)

# Link libraries
target_link_libraries(my_app PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cpprestsdk::cpprest
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto)

# Set C++ standard
target_compile_features(my_app PRIVATE cxx_std_17)
)" << std::endl;

    std::cout << "\n4. Build process with Conan:" << std::endl;
    std::cout << "mkdir build && cd build" << std::endl;
    std::cout << "conan install .. --build=missing" << std::endl;
    std::cout << "cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake" << std::endl;
    std::cout << "cmake --build ." << std::endl;
}

void demonstrate_conan_advanced() {
    std::cout << "\n=== Advanced Conan Usage ===" << std::endl;
    
    std::cout << "\n1. conanfile.py (Python-based):" << std::endl;
    std::cout << R"(
from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout

class MyAppConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    
    def requirements(self):
        self.requires("nlohmann_json/3.11.2")
        self.requires("spdlog/1.12.0")
        if self.settings.os != "Windows":
            self.requires("openssl/3.1.1")
    
    def build_requirements(self):
        self.tool_requires("cmake/3.25.3")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
    
    def configure(self):
        if self.settings.compiler == "Visual Studio":
            self.options["boost"].shared = True
)" << std::endl;

    std::cout << "\n2. Profiles for different configurations:" << std::endl;
    std::cout << R"(
# profiles/debug
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Debug

[options]
*:shared=False

# profiles/release
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Release

[options]
*:shared=False
)" << std::endl;

    std::cout << "\n3. Using profiles:" << std::endl;
    std::cout << "conan install . --profile=profiles/debug --build=missing" << std::endl;
    std::cout << "conan install . --profile=profiles/release --build=missing" << std::endl;
    
    std::cout << "\n4. Cross-compilation with Conan:" << std::endl;
    std::cout << "conan install . --profile:build=default --profile:host=arm64" << std::endl;
}

// =============================================================================
// VCPKG PACKAGE MANAGER
// =============================================================================

void demonstrate_vcpkg_basics() {
    std::cout << "\n=== vcpkg Package Manager ===" << std::endl;
    
    std::cout << "\nvcpkg is Microsoft's C++ package manager:" << std::endl;
    std::cout << "• Source-based builds by default" << std::endl;
    std::cout << "• Excellent Visual Studio integration" << std::endl;
    std::cout << "• CMake integration built-in" << std::endl;
    std::cout << "• Manifest mode for project-specific dependencies" << std::endl;
    
    std::cout << "\n1. Installation:" << std::endl;
    std::cout << "git clone https://github.com/Microsoft/vcpkg.git" << std::endl;
    std::cout << "cd vcpkg" << std::endl;
    std::cout << "./bootstrap-vcpkg.sh    # Linux/macOS" << std::endl;
    std::cout << ".\\bootstrap-vcpkg.bat  # Windows" << std::endl;
    
    std::cout << "\n2. vcpkg.json manifest file:" << std::endl;
    std::cout << R"(
{
  "name": "my-application",
  "version": "1.0.0",
  "dependencies": [
    "nlohmann-json",
    "spdlog",
    "cpprestsdk",
    {
      "name": "boost",
      "features": ["system", "filesystem"]
    },
    {
      "name": "openssl",
      "platform": "!windows"
    }
  ],
  "builtin-baseline": "2023.08.09"
}
)" << std::endl;

    std::cout << "\n3. CMakeLists.txt with vcpkg:" << std::endl;
    std::cout << R"(
cmake_minimum_required(VERSION 3.16)
project(VcpkgExample)

# Find packages installed by vcpkg
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)

# Create executable
add_executable(my_app main.cpp)

# Link libraries
target_link_libraries(my_app PRIVATE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cpprestsdk::cpprest
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto)

# Set C++ standard
target_compile_features(my_app PRIVATE cxx_std_17)
)" << std::endl;

    std::cout << "\n4. Build process with vcpkg:" << std::endl;
    std::cout << "cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake" << std::endl;
    std::cout << "cmake --build build" << std::endl;
    
    std::cout << "\nOr with environment variable:" << std::endl;
    std::cout << "export VCPKG_ROOT=/path/to/vcpkg" << std::endl;
    std::cout << "cmake -B build" << std::endl;
    std::cout << "cmake --build build" << std::endl;
}

void demonstrate_vcpkg_advanced() {
    std::cout << "\n=== Advanced vcpkg Usage ===" << std::endl;
    
    std::cout << "\n1. Triplets for different configurations:" << std::endl;
    std::cout << R"(
# Custom triplet: x64-linux-dynamic.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Usage:
vcpkg install --triplet x64-linux-dynamic
)" << std::endl;

    std::cout << "\n2. vcpkg-configuration.json for registries:" << std::endl;
    std::cout << R"(
{
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/Microsoft/vcpkg",
    "baseline": "2023.08.09"
  },
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/company/vcpkg-registry",
      "baseline": "latest",
      "packages": ["company-*"]
    }
  ]
}
)" << std::endl;

    std::cout << "\n3. Feature selection in vcpkg.json:" << std::endl;
    std::cout << R"(
{
  "dependencies": [
    {
      "name": "opencv",
      "features": ["contrib", "nonfree"]
    },
    {
      "name": "qt5",
      "features": ["core", "widgets", "network"]
    }
  ]
}
)" << std::endl;

    std::cout << "\n4. Cross-compilation with vcpkg:" << std::endl;
    std::cout << "vcpkg install --triplet arm64-linux" << std::endl;
    std::cout << "cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=arm64-linux" << std::endl;
}

// =============================================================================
// COMPARISON AND BEST PRACTICES
// =============================================================================

void demonstrate_conan_vs_vcpkg() {
    std::cout << "\n=== Conan vs vcpkg Comparison ===" << std::endl;
    
    std::cout << "\n┌─────────────────┬─────────────────┬─────────────────┐" << std::endl;
    std::cout << "│ Feature         │ Conan           │ vcpkg           │" << std::endl;
    std::cout << "├─────────────────┼─────────────────┼─────────────────┤" << std::endl;
    std::cout << "│ Package Format  │ Binary/Source   │ Source-based    │" << std::endl;
    std::cout << "│ Build Speed     │ Faster (binary) │ Slower (source) │" << std::endl;
    std::cout << "│ Customization   │ High            │ Medium          │" << std::endl;
    std::cout << "│ VS Integration  │ Good            │ Excellent       │" << std::endl;
    std::cout << "│ Cross-platform  │ Excellent       │ Good            │" << std::endl;
    std::cout << "│ Package Count   │ ~1000           │ ~1500           │" << std::endl;
    std::cout << "│ Learning Curve  │ Steep           │ Moderate        │" << std::endl;
    std::cout << "│ Dependency Mgmt │ Advanced        │ Simple          │" << std::endl;
    std::cout << "└─────────────────┴─────────────────┴─────────────────┘" << std::endl;
    
    std::cout << "\nWhen to use Conan:" << std::endl;
    std::cout << "• Complex dependency requirements" << std::endl;
    std::cout << "• Need for binary packages" << std::endl;
    std::cout << "• Custom package repositories" << std::endl;
    std::cout << "• Advanced configuration options" << std::endl;
    std::cout << "• CI/CD with multiple configurations" << std::endl;
    
    std::cout << "\nWhen to use vcpkg:" << std::endl;
    std::cout << "• Visual Studio development" << std::endl;
    std::cout << "• Simple dependency management" << std::endl;
    std::cout << "• Source-based customization needed" << std::endl;
    std::cout << "• Microsoft ecosystem integration" << std::endl;
    std::cout << "• Getting started with package management" << std::endl;
}

void demonstrate_integration_patterns() {
    std::cout << "\n=== Package Manager Integration Patterns ===" << std::endl;
    
    std::cout << "\n1. Hybrid Approach:" << std::endl;
    std::cout << R"(
# Use both package managers for different purposes
# vcpkg for core dependencies
# Conan for specialized libraries

# CMakeLists.txt
find_package(nlohmann_json CONFIG REQUIRED)  # vcpkg
find_package(spdlog REQUIRED)                # Conan
)" << std::endl;

    std::cout << "\n2. Fallback Strategy:" << std::endl;
    std::cout << R"(
# Try vcpkg first, fallback to system packages
find_package(Boost CONFIG QUIET)
if(NOT Boost_FOUND)
    find_package(Boost REQUIRED COMPONENTS system filesystem)
endif()
)" << std::endl;

    std::cout << "\n3. Conditional Package Management:" << std::endl;
    std::cout << R"(
# CMakeLists.txt
option(USE_CONAN "Use Conan for dependencies" OFF)
option(USE_VCPKG "Use vcpkg for dependencies" OFF)

if(USE_CONAN)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
elseif(USE_VCPKG)
    # vcpkg toolchain handled via command line
endif()

# Find packages (same regardless of source)
find_package(nlohmann_json REQUIRED)
)" << std::endl;

    std::cout << "\n4. Docker Integration:" << std::endl;
    std::cout << R"(
# Dockerfile with vcpkg
FROM ubuntu:20.04
RUN apt-get update && apt-get install -y git cmake build-essential
RUN git clone https://github.com/Microsoft/vcpkg.git
RUN ./vcpkg/bootstrap-vcpkg.sh
ENV VCPKG_ROOT=/vcpkg
WORKDIR /app
COPY vcpkg.json .
RUN cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
)" << std::endl;
}

void demonstrate_troubleshooting() {
    std::cout << "\n=== Package Manager Troubleshooting ===" << std::endl;
    
    std::cout << "\nCommon Conan issues:" << std::endl;
    std::cout << "1. Profile conflicts:" << std::endl;
    std::cout << "   • Check: conan profile show default" << std::endl;
    std::cout << "   • Fix: conan profile detect --force" << std::endl;
    
    std::cout << "\n2. Missing packages:" << std::endl;
    std::cout << "   • Use: --build=missing flag" << std::endl;
    std::cout << "   • Or: conan create . for custom packages" << std::endl;
    
    std::cout << "\n3. Version conflicts:" << std::endl;
    std::cout << "   • Check: conan graph info ." << std::endl;
    std::cout << "   • Override: [requires] package/version@override" << std::endl;
    
    std::cout << "\nCommon vcpkg issues:" << std::endl;
    std::cout << "1. Toolchain not found:" << std::endl;
    std::cout << "   • Set VCPKG_ROOT environment variable" << std::endl;
    std::cout << "   • Or use full path to vcpkg.cmake" << std::endl;
    
    std::cout << "\n2. Package not found:" << std::endl;
    std::cout << "   • Check: vcpkg list" << std::endl;
    std::cout << "   • Install: vcpkg install package" << std::endl;
    
    std::cout << "\n3. Build failures:" << std::endl;
    std::cout << "   • Check logs in buildtrees/" << std::endl;
    std::cout << "   • Try different triplet" << std::endl;
    
    std::cout << "\nDebugging commands:" << std::endl;
    std::cout << "# Conan" << std::endl;
    std::cout << "conan graph info . --build=missing" << std::endl;
    std::cout << "conan profile show default" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "# vcpkg" << std::endl;
    std::cout << "vcpkg list" << std::endl;
    std::cout << "vcpkg integrate project" << std::endl;
}

// =============================================================================
// PRACTICAL DEMONSTRATION
// =============================================================================

void demonstrate_practical_usage() {
    std::cout << "\n=== Practical Package Usage Example ===" << std::endl;
    
    // Demonstrate usage of typical packages
    json_example::JsonLibrary::demonstrate_usage();
    http_example::HttpClient::demonstrate_usage();
    logging_example::Logger::demonstrate_usage();
    
    std::cout << "\nThis demonstrates typical usage of common C++ packages:" << std::endl;
    std::cout << "• JSON library (nlohmann/json)" << std::endl;
    std::cout << "• HTTP client (cpprestsdk)" << std::endl;
    std::cout << "• Logging library (spdlog)" << std::endl;
    
    std::cout << "\nWith package managers, adding these dependencies is simple:" << std::endl;
    std::cout << "1. Add to conanfile.txt or vcpkg.json" << std::endl;
    std::cout << "2. Run package manager install" << std::endl;
    std::cout << "3. Use find_package() in CMake" << std::endl;
    std::cout << "4. Link with target_link_libraries()" << std::endl;
}

int main() {
    std::cout << "CONAN AND VCPKG PACKAGE MANAGERS\n";
    std::cout << "=================================\n";
    
    demonstrate_conan_basics();
    demonstrate_conan_advanced();
    demonstrate_vcpkg_basics();
    demonstrate_vcpkg_advanced();
    demonstrate_conan_vs_vcpkg();
    demonstrate_integration_patterns();
    demonstrate_troubleshooting();
    demonstrate_practical_usage();
    
    std::cout << "\nKey Package Management Concepts:" << std::endl;
    std::cout << "• Package managers simplify dependency management" << std::endl;
    std::cout << "• Conan offers binary packages and advanced features" << std::endl;
    std::cout << "• vcpkg provides excellent Visual Studio integration" << std::endl;
    std::cout << "• Both integrate well with CMake" << std::endl;
    std::cout << "• Choose based on project needs and team preferences" << std::endl;
    std::cout << "• Hybrid approaches are possible for complex projects" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Package managers eliminate manual dependency management
- Conan excels with binary packages and complex configurations
- vcpkg provides seamless Visual Studio integration
- Both support cross-platform development
- CMake integration is excellent for both tools
- Manifest files enable reproducible builds
- Version management prevents dependency conflicts
- Docker integration enables consistent environments
- Troubleshooting requires understanding each tool's approach
*/
