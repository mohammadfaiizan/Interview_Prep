/*
 * LINUX SYSTEM LEVEL C++ - MEMORY MAPPING AND SHARED MEMORY
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread mmap_shared_memory.cpp -o mmap_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/wait.h>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC MEMORY MAPPING
// =============================================================================

void demonstrate_basic_mmap() {
    SECTION("Basic Memory Mapping");
    
    const char* filename = "/tmp/mmap_basic.txt";
    const size_t file_size = 4096;
    
    // Create and initialize file
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "File creation failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Extend file to desired size
    if (ftruncate(fd, file_size) == -1) {
        std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    // Map file into memory
    void* mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "File mapped at address: " << mapped << std::endl;
    
    // Write to memory (which writes to file)
    char* data = static_cast<char*>(mapped);
    strcpy(data, "Hello, memory mapped file!");
    
    std::cout << "Written to memory: " << data << std::endl;
    
    // Ensure data is written to disk
    if (msync(mapped, strlen(data), MS_SYNC) == -1) {
        std::cerr << "msync failed: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Data synchronized to disk" << std::endl;
    }
    
    // Unmap memory
    if (munmap(mapped, file_size) == -1) {
        std::cerr << "munmap failed: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Memory unmapped successfully" << std::endl;
    }
    
    close(fd);
    
    // Verify file contents
    fd = open(filename, O_RDONLY);
    if (fd != -1) {
        char buffer[256];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "File contents: " << buffer << std::endl;
        }
        close(fd);
    }
    
    unlink(filename);
}

// =============================================================================
// 2. ANONYMOUS MEMORY MAPPING
// =============================================================================

void demonstrate_anonymous_mmap() {
    SECTION("Anonymous Memory Mapping");
    
    const size_t size = 4096;
    
    // Create anonymous mapping
    void* mapped = mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                       MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Anonymous mmap failed: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "Anonymous memory mapped at: " << mapped << std::endl;
    
    // Use the memory
    char* data = static_cast<char*>(mapped);
    strcpy(data, "Hello, anonymous memory!");
    
    std::cout << "Written to anonymous memory: " << data << std::endl;
    
    // Get memory statistics
    std::cout << "Memory page size: " << getpagesize() << " bytes" << std::endl;
    
    // Unmap memory
    if (munmap(mapped, size) == -1) {
        std::cerr << "munmap failed: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Anonymous memory unmapped" << std::endl;
    }
}

// =============================================================================
// 3. SHARED MEMORY BETWEEN PROCESSES
// =============================================================================

struct shared_data {
    int counter;
    char message[256];
    bool ready;
    std::chrono::steady_clock::time_point timestamp;
};

void demonstrate_shared_memory_processes() {
    SECTION("Shared Memory Between Processes");
    
    const size_t size = sizeof(shared_data);
    
    // Create shared memory mapping
    void* mapped = mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                       MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Shared mmap failed: " << strerror(errno) << std::endl;
        return;
    }
    
    shared_data* data = static_cast<shared_data*>(mapped);
    
    // Initialize shared data
    data->counter = 0;
    data->ready = false;
    strcpy(data->message, "Initial message");
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        munmap(mapped, size);
        return;
    }
    
    if (pid == 0) {
        // Child process
        std::cout << "Child: Waiting for parent to update shared memory..." << std::endl;
        
        while (!data->ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "Child: Shared data received!" << std::endl;
        std::cout << "  Counter: " << data->counter << std::endl;
        std::cout << "  Message: " << data->message << std::endl;
        
        // Child updates data
        data->counter *= 2;
        strcpy(data->message, "Updated by child");
        
        exit(0);
    } else {
        // Parent process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Parent: Updating shared memory..." << std::endl;
        data->counter = 42;
        strcpy(data->message, "Hello from parent!");
        data->timestamp = std::chrono::steady_clock::now();
        data->ready = true;
        
        wait(nullptr); // Wait for child to complete
        
        std::cout << "Parent: Child has processed data" << std::endl;
        std::cout << "  Final counter: " << data->counter << std::endl;
        std::cout << "  Final message: " << data->message << std::endl;
        
        munmap(mapped, size);
    }
}

// =============================================================================
// 4. MEMORY PROTECTION AND PERMISSIONS
// =============================================================================

void demonstrate_memory_protection() {
    SECTION("Memory Protection and Permissions");
    
    const size_t size = 4096;
    
    // Create read-only mapping
    void* readonly_mem = mmap(nullptr, size, PROT_READ, 
                             MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (readonly_mem == MAP_FAILED) {
        std::cerr << "Read-only mmap failed: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "Created read-only memory mapping" << std::endl;
    
    // Try to write (this should fail)
    std::cout << "Attempting to write to read-only memory..." << std::endl;
    // Note: This would cause a segmentation fault in practice
    // char* data = static_cast<char*>(readonly_mem);
    // strcpy(data, "This will fail");
    
    // Change protection to read-write
    if (mprotect(readonly_mem, size, PROT_READ | PROT_WRITE) == 0) {
        std::cout << "Changed protection to read-write" << std::endl;
        
        char* data = static_cast<char*>(readonly_mem);
        strcpy(data, "Now I can write!");
        std::cout << "Successfully wrote: " << data << std::endl;
    } else {
        std::cerr << "mprotect failed: " << strerror(errno) << std::endl;
    }
    
    // Change back to read-only
    if (mprotect(readonly_mem, size, PROT_READ) == 0) {
        std::cout << "Changed protection back to read-only" << std::endl;
    }
    
    munmap(readonly_mem, size);
}

// =============================================================================
// 5. MEMORY MAPPING LARGE FILES
// =============================================================================

void demonstrate_large_file_mapping() {
    SECTION("Large File Memory Mapping");
    
    const char* filename = "/tmp/large_file.dat";
    const size_t file_size = 1024 * 1024; // 1MB
    
    // Create large file
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Large file creation failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (ftruncate(fd, file_size) == -1) {
        std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    // Map entire file
    void* mapped = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        std::cerr << "Large file mmap failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "Mapped " << file_size << " bytes" << std::endl;
    
    // Write pattern to file
    char* data = static_cast<char*>(mapped);
    for (size_t i = 0; i < file_size; i += 4096) {
        snprintf(data + i, 100, "Page %zu: Hello from large file mapping!\n", i / 4096);
    }
    
    std::cout << "Written pattern to large file" << std::endl;
    
    // Read back some data
    std::cout << "First page: " << std::string(data, 50) << std::endl;
    std::cout << "Last page: " << std::string(data + file_size - 4096, 50) << std::endl;
    
    // Advise kernel about access pattern
    if (madvise(mapped, file_size, MADV_SEQUENTIAL) == 0) {
        std::cout << "Advised kernel about sequential access pattern" << std::endl;
    }
    
    // Synchronize part of the file
    if (msync(mapped, 4096, MS_ASYNC) == 0) {
        std::cout << "Asynchronously synchronized first page" << std::endl;
    }
    
    munmap(mapped, file_size);
    close(fd);
    unlink(filename);
}

// =============================================================================
// 6. MEMORY MAPPING PERFORMANCE
// =============================================================================

void performance_comparison() {
    SECTION("Memory Mapping vs Traditional I/O Performance");
    
    const char* filename = "/tmp/perf_test.dat";
    const size_t data_size = 1024 * 1024; // 1MB
    std::vector<char> test_data(data_size, 'A');
    
    // Test 1: Traditional write/read
    auto start = std::chrono::high_resolution_clock::now();
    
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    write(fd, test_data.data(), data_size);
    close(fd);
    
    fd = open(filename, O_RDONLY);
    std::vector<char> read_buffer(data_size);
    read(fd, read_buffer.data(), data_size);
    close(fd);
    
    auto traditional_time = std::chrono::high_resolution_clock::now() - start;
    
    // Test 2: Memory mapping
    start = std::chrono::high_resolution_clock::now();
    
    fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    ftruncate(fd, data_size);
    
    void* mapped = mmap(nullptr, data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memcpy(mapped, test_data.data(), data_size);
    msync(mapped, data_size, MS_SYNC);
    
    std::vector<char> mmap_buffer(data_size);
    memcpy(mmap_buffer.data(), mapped, data_size);
    
    munmap(mapped, data_size);
    close(fd);
    
    auto mmap_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance Results (1MB data):" << std::endl;
    std::cout << "Traditional I/O: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(traditional_time).count() 
              << " μs" << std::endl;
    std::cout << "Memory mapping: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(mmap_time).count() 
              << " μs" << std::endl;
    
    double speedup = static_cast<double>(traditional_time.count()) / mmap_time.count();
    std::cout << "Memory mapping speedup: " << speedup << "x" << std::endl;
    
    unlink(filename);
}

// =============================================================================
// 7. RAII MEMORY MAPPING CLASS
// =============================================================================

class MemoryMap {
private:
    void* addr_;
    size_t size_;
    int fd_;
    
public:
    MemoryMap(const std::string& filename, size_t size, int prot = PROT_READ | PROT_WRITE)
        : addr_(nullptr), size_(size), fd_(-1) {
        
        fd_ = open(filename.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd_ == -1) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        
        if (ftruncate(fd_, size) == -1) {
            close(fd_);
            throw std::runtime_error("Failed to resize file: " + std::string(strerror(errno)));
        }
        
        addr_ = mmap(nullptr, size, prot, MAP_SHARED, fd_, 0);
        if (addr_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("Failed to map memory: " + std::string(strerror(errno)));
        }
    }
    
    ~MemoryMap() {
        if (addr_ != nullptr) {
            munmap(addr_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
    }
    
    // Delete copy operations
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator=(const MemoryMap&) = delete;
    
    // Move operations
    MemoryMap(MemoryMap&& other) noexcept 
        : addr_(other.addr_), size_(other.size_), fd_(other.fd_) {
        other.addr_ = nullptr;
        other.fd_ = -1;
    }
    
    MemoryMap& operator=(MemoryMap&& other) noexcept {
        if (this != &other) {
            if (addr_ != nullptr) {
                munmap(addr_, size_);
            }
            if (fd_ != -1) {
                close(fd_);
            }
            
            addr_ = other.addr_;
            size_ = other.size_;
            fd_ = other.fd_;
            
            other.addr_ = nullptr;
            other.fd_ = -1;
        }
        return *this;
    }
    
    void* data() { return addr_; }
    const void* data() const { return addr_; }
    size_t size() const { return size_; }
    
    void sync(bool async = false) {
        if (msync(addr_, size_, async ? MS_ASYNC : MS_SYNC) == -1) {
            throw std::runtime_error("Failed to sync memory: " + std::string(strerror(errno)));
        }
    }
    
    void protect(int prot) {
        if (mprotect(addr_, size_, prot) == -1) {
            throw std::runtime_error("Failed to change protection: " + std::string(strerror(errno)));
        }
    }
};

void demonstrate_raii_mmap() {
    SECTION("RAII Memory Mapping");
    
    const char* filename = "/tmp/raii_mmap_test.dat";
    
    try {
        MemoryMap mmap(filename, 4096);
        
        char* data = static_cast<char*>(mmap.data());
        strcpy(data, "Hello from RAII memory mapping!");
        
        std::cout << "Written to RAII memory map: " << data << std::endl;
        
        mmap.sync(); // Synchronize to disk
        std::cout << "Memory synchronized to disk" << std::endl;
        
        // Memory automatically unmapped when mmap goes out of scope
    } catch (const std::exception& e) {
        std::cerr << "RAII mmap error: " << e.what() << std::endl;
    }
    
    unlink(filename);
}

int main() {
    std::cout << "LINUX MEMORY MAPPING AND SHARED MEMORY\n";
    std::cout << "======================================\n";
    
    demonstrate_basic_mmap();
    demonstrate_anonymous_mmap();
    demonstrate_shared_memory_processes();
    demonstrate_memory_protection();
    demonstrate_large_file_mapping();
    performance_comparison();
    demonstrate_raii_mmap();
    
    std::cout << "\nKey Memory Mapping Concepts:" << std::endl;
    std::cout << "- mmap() maps files or memory into process address space" << std::endl;
    std::cout << "- Shared mappings enable IPC between processes" << std::endl;
    std::cout << "- Memory protection can be changed with mprotect()" << std::endl;
    std::cout << "- msync() controls when changes are written to disk" << std::endl;
    std::cout << "- madvise() provides hints to the kernel about access patterns" << std::endl;
    std::cout << "- RAII ensures proper cleanup of memory mappings" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Memory mapping provides efficient file I/O and IPC
- Shared mappings enable zero-copy data sharing between processes
- Anonymous mappings provide fast temporary memory allocation
- Memory protection enables security and debugging features
- Performance benefits depend on access patterns and file size
- Proper synchronization is crucial for shared memory
- RAII principles ensure resource cleanup and exception safety
- mmap is particularly effective for large files and random access
*/
