/*
 * LINUX SYSTEM LEVEL C++ - FILE DESCRIPTOR I/O
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra file_descriptor_io.cpp -o file_descriptor_io
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC FILE OPERATIONS
// =============================================================================

void demonstrate_basic_file_operations() {
    SECTION("Basic File Operations");
    
    const char* filename = "/tmp/test_file.txt";
    const char* data = "Hello, Linux file I/O!";
    
    // Create and write to file
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return;
    }
    
    ssize_t bytes_written = write(fd, data, strlen(data));
    if (bytes_written == -1) {
        std::cerr << "Error writing to file: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "Written " << bytes_written << " bytes to " << filename << std::endl;
    close(fd);
    
    // Read from file
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error opening file for reading: " << strerror(errno) << std::endl;
        return;
    }
    
    char buffer[256];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        std::cerr << "Error reading from file: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    std::cout << "Read " << bytes_read << " bytes: " << buffer << std::endl;
    close(fd);
    
    // Clean up
    unlink(filename);
}

// =============================================================================
// 2. FILE DESCRIPTOR FLAGS AND MODES
// =============================================================================

void demonstrate_fd_flags() {
    SECTION("File Descriptor Flags and Modes");
    
    const char* filename = "/tmp/flags_test.txt";
    
    // Different open flags
    std::cout << "Open flags demonstration:" << std::endl;
    
    // O_APPEND - append mode
    int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd != -1) {
        write(fd, "Line 1\n", 7);
        write(fd, "Line 2\n", 7);
        close(fd);
        std::cout << "Created file with append mode" << std::endl;
    }
    
    // O_TRUNC - truncate file
    fd = open(filename, O_WRONLY | O_TRUNC);
    if (fd != -1) {
        write(fd, "Truncated\n", 10);
        close(fd);
        std::cout << "File truncated and rewritten" << std::endl;
    }
    
    // Get file descriptor flags
    fd = open(filename, O_RDWR);
    if (fd != -1) {
        int flags = fcntl(fd, F_GETFL);
        if (flags != -1) {
            std::cout << "File descriptor flags: ";
            if (flags & O_RDONLY) std::cout << "O_RDONLY ";
            if (flags & O_WRONLY) std::cout << "O_WRONLY ";
            if (flags & O_RDWR) std::cout << "O_RDWR ";
            if (flags & O_APPEND) std::cout << "O_APPEND ";
            if (flags & O_NONBLOCK) std::cout << "O_NONBLOCK ";
            std::cout << std::endl;
        }
        
        // Set non-blocking mode
        int new_flags = flags | O_NONBLOCK;
        if (fcntl(fd, F_SETFL, new_flags) == 0) {
            std::cout << "Set non-blocking mode" << std::endl;
        }
        
        close(fd);
    }
    
    unlink(filename);
}

// =============================================================================
// 3. FILE POSITIONING AND SEEKING
// =============================================================================

void demonstrate_file_positioning() {
    SECTION("File Positioning and Seeking");
    
    const char* filename = "/tmp/seek_test.txt";
    const char* data = "0123456789ABCDEFGHIJ";
    
    // Create test file
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error creating file: " << strerror(errno) << std::endl;
        return;
    }
    
    write(fd, data, strlen(data));
    
    // Seek to beginning
    off_t pos = lseek(fd, 0, SEEK_SET);
    std::cout << "Seeked to position: " << pos << std::endl;
    
    char buffer[5];
    read(fd, buffer, 4);
    buffer[4] = '\0';
    std::cout << "Read from beginning: " << buffer << std::endl;
    
    // Seek to middle
    pos = lseek(fd, 10, SEEK_SET);
    std::cout << "Seeked to position: " << pos << std::endl;
    
    read(fd, buffer, 4);
    buffer[4] = '\0';
    std::cout << "Read from middle: " << buffer << std::endl;
    
    // Seek to end
    pos = lseek(fd, 0, SEEK_END);
    std::cout << "File size: " << pos << " bytes" << std::endl;
    
    // Seek relative to current position
    lseek(fd, -5, SEEK_CUR);
    read(fd, buffer, 4);
    buffer[4] = '\0';
    std::cout << "Read from near end: " << buffer << std::endl;
    
    close(fd);
    unlink(filename);
}

// =============================================================================
// 4. DIRECTORY OPERATIONS
// =============================================================================

void demonstrate_directory_operations() {
    SECTION("Directory Operations");
    
    const char* dirname = "/tmp/test_directory";
    
    // Create directory
    if (mkdir(dirname, 0755) == 0) {
        std::cout << "Created directory: " << dirname << std::endl;
    } else {
        std::cerr << "Error creating directory: " << strerror(errno) << std::endl;
        return;
    }
    
    // Open directory
    DIR* dir = opendir(dirname);
    if (dir == nullptr) {
        std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
        rmdir(dirname);
        return;
    }
    
    std::cout << "Directory contents:" << std::endl;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::cout << "  " << entry->d_name;
        switch (entry->d_type) {
            case DT_REG: std::cout << " (regular file)"; break;
            case DT_DIR: std::cout << " (directory)"; break;
            case DT_LNK: std::cout << " (symbolic link)"; break;
            default: std::cout << " (other)"; break;
        }
        std::cout << std::endl;
    }
    
    closedir(dir);
    
    // Create a file in the directory
    std::string filepath = std::string(dirname) + "/test_file.txt";
    int fd = open(filepath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd != -1) {
        write(fd, "test content", 12);
        close(fd);
        std::cout << "Created file in directory" << std::endl;
    }
    
    // List directory again
    dir = opendir(dirname);
    if (dir != nullptr) {
        std::cout << "Updated directory contents:" << std::endl;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                std::cout << "  " << entry->d_name << std::endl;
            }
        }
        closedir(dir);
    }
    
    // Clean up
    unlink(filepath.c_str());
    rmdir(dirname);
}

// =============================================================================
// 5. FILE METADATA AND STATISTICS
// =============================================================================

void demonstrate_file_metadata() {
    SECTION("File Metadata and Statistics");
    
    const char* filename = "/tmp/metadata_test.txt";
    
    // Create test file
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        std::cerr << "Error creating file: " << strerror(errno) << std::endl;
        return;
    }
    
    const char* data = "This is test data for metadata demonstration.";
    write(fd, data, strlen(data));
    close(fd);
    
    // Get file statistics
    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        std::cout << "File statistics for " << filename << ":" << std::endl;
        std::cout << "  Size: " << file_stat.st_size << " bytes" << std::endl;
        std::cout << "  Mode: " << std::oct << file_stat.st_mode << std::dec << std::endl;
        std::cout << "  UID: " << file_stat.st_uid << std::endl;
        std::cout << "  GID: " << file_stat.st_gid << std::endl;
        std::cout << "  Links: " << file_stat.st_nlink << std::endl;
        std::cout << "  Inode: " << file_stat.st_ino << std::endl;
        
        // File type
        std::cout << "  Type: ";
        if (S_ISREG(file_stat.st_mode)) std::cout << "Regular file";
        else if (S_ISDIR(file_stat.st_mode)) std::cout << "Directory";
        else if (S_ISLNK(file_stat.st_mode)) std::cout << "Symbolic link";
        else if (S_ISBLK(file_stat.st_mode)) std::cout << "Block device";
        else if (S_ISCHR(file_stat.st_mode)) std::cout << "Character device";
        else if (S_ISFIFO(file_stat.st_mode)) std::cout << "FIFO";
        else if (S_ISSOCK(file_stat.st_mode)) std::cout << "Socket";
        std::cout << std::endl;
        
        // Permissions
        std::cout << "  Permissions: ";
        std::cout << (file_stat.st_mode & S_IRUSR ? "r" : "-");
        std::cout << (file_stat.st_mode & S_IWUSR ? "w" : "-");
        std::cout << (file_stat.st_mode & S_IXUSR ? "x" : "-");
        std::cout << (file_stat.st_mode & S_IRGRP ? "r" : "-");
        std::cout << (file_stat.st_mode & S_IWGRP ? "w" : "-");
        std::cout << (file_stat.st_mode & S_IXGRP ? "x" : "-");
        std::cout << (file_stat.st_mode & S_IROTH ? "r" : "-");
        std::cout << (file_stat.st_mode & S_IWOTH ? "w" : "-");
        std::cout << (file_stat.st_mode & S_IXOTH ? "x" : "-");
        std::cout << std::endl;
        
        // Timestamps
        std::cout << "  Access time: " << ctime(&file_stat.st_atime);
        std::cout << "  Modify time: " << ctime(&file_stat.st_mtime);
        std::cout << "  Change time: " << ctime(&file_stat.st_ctime);
    }
    
    unlink(filename);
}

// =============================================================================
// 6. ADVANCED I/O OPERATIONS
// =============================================================================

void demonstrate_advanced_io() {
    SECTION("Advanced I/O Operations");
    
    const char* filename = "/tmp/advanced_io_test.txt";
    
    // Vectored I/O (readv/writev)
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error creating file: " << strerror(errno) << std::endl;
        return;
    }
    
    // Write using writev
    const char* msg1 = "Hello, ";
    const char* msg2 = "vectored ";
    const char* msg3 = "I/O!";
    
    struct iovec write_iov[3];
    write_iov[0].iov_base = const_cast<char*>(msg1);
    write_iov[0].iov_len = strlen(msg1);
    write_iov[1].iov_base = const_cast<char*>(msg2);
    write_iov[1].iov_len = strlen(msg2);
    write_iov[2].iov_base = const_cast<char*>(msg3);
    write_iov[2].iov_len = strlen(msg3);
    
    ssize_t bytes_written = writev(fd, write_iov, 3);
    std::cout << "Wrote " << bytes_written << " bytes using writev" << std::endl;
    
    // Read using readv
    lseek(fd, 0, SEEK_SET);
    char buf1[10], buf2[10], buf3[10];
    
    struct iovec read_iov[3];
    read_iov[0].iov_base = buf1;
    read_iov[0].iov_len = 7;
    read_iov[1].iov_base = buf2;
    read_iov[1].iov_len = 9;
    read_iov[2].iov_base = buf3;
    read_iov[2].iov_len = 4;
    
    ssize_t bytes_read = readv(fd, read_iov, 3);
    std::cout << "Read " << bytes_read << " bytes using readv" << std::endl;
    
    buf1[7] = buf2[9] = buf3[4] = '\0';
    std::cout << "Read data: '" << buf1 << "' + '" << buf2 << "' + '" << buf3 << "'" << std::endl;
    
    close(fd);
    unlink(filename);
}

// =============================================================================
// 7. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison");
    
    const char* filename = "/tmp/perf_test.txt";
    const size_t data_size = 1024 * 1024; // 1MB
    std::vector<char> data(data_size, 'A');
    
    // Test 1: Single large write
    auto start = std::chrono::high_resolution_clock::now();
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(fd, data.data(), data_size);
    close(fd);
    auto single_write_time = std::chrono::high_resolution_clock::now() - start;
    
    // Test 2: Multiple small writes
    start = std::chrono::high_resolution_clock::now();
    fd = open(filename, O_WRONLY | O_TRUNC);
    for (size_t i = 0; i < data_size; i += 1024) {
        write(fd, data.data() + i, 1024);
    }
    close(fd);
    auto multiple_write_time = std::chrono::high_resolution_clock::now() - start;
    
    // Test 3: Buffered vs unbuffered
    start = std::chrono::high_resolution_clock::now();
    fd = open(filename, O_WRONLY | O_TRUNC | O_SYNC); // Synchronous writes
    write(fd, data.data(), data_size);
    close(fd);
    auto sync_write_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Performance Results (1MB data):" << std::endl;
    std::cout << "Single write: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(single_write_time).count() 
              << " μs" << std::endl;
    std::cout << "Multiple writes: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(multiple_write_time).count() 
              << " μs" << std::endl;
    std::cout << "Synchronous write: " 
              << std::chrono::duration_cast<std::chrono::microseconds>(sync_write_time).count() 
              << " μs" << std::endl;
    
    unlink(filename);
}

// =============================================================================
// 8. ERROR HANDLING AND BEST PRACTICES
// =============================================================================

class FileDescriptor {
private:
    int fd_;
    
public:
    explicit FileDescriptor(int fd = -1) : fd_(fd) {}
    
    ~FileDescriptor() {
        if (fd_ != -1) {
            close(fd_);
        }
    }
    
    // Move semantics
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            if (fd_ != -1) {
                close(fd_);
            }
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    
    // Delete copy operations
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    
    int get() const { return fd_; }
    bool valid() const { return fd_ != -1; }
    
    int release() {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }
};

void demonstrate_raii_fd() {
    SECTION("RAII File Descriptor Management");
    
    const char* filename = "/tmp/raii_test.txt";
    
    {
        FileDescriptor fd(open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644));
        if (!fd.valid()) {
            std::cerr << "Error opening file: " << strerror(errno) << std::endl;
            return;
        }
        
        const char* data = "RAII managed file descriptor";
        if (write(fd.get(), data, strlen(data)) == -1) {
            std::cerr << "Error writing: " << strerror(errno) << std::endl;
            return;
        }
        
        std::cout << "File written successfully with RAII" << std::endl;
        // File descriptor automatically closed when fd goes out of scope
    }
    
    // Verify file was written
    FileDescriptor read_fd(open(filename, O_RDONLY));
    if (read_fd.valid()) {
        char buffer[100];
        ssize_t bytes_read = read(read_fd.get(), buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Read back: " << buffer << std::endl;
        }
    }
    
    unlink(filename);
}

int main() {
    std::cout << "LINUX FILE DESCRIPTOR I/O TUTORIAL\n";
    std::cout << "==================================\n";
    
    demonstrate_basic_file_operations();
    demonstrate_fd_flags();
    demonstrate_file_positioning();
    demonstrate_directory_operations();
    demonstrate_file_metadata();
    demonstrate_advanced_io();
    performance_comparison();
    demonstrate_raii_fd();
    
    std::cout << "\nKey Concepts:" << std::endl;
    std::cout << "- File descriptors are integers representing open files" << std::endl;
    std::cout << "- Always check return values for error conditions" << std::endl;
    std::cout << "- Use RAII for automatic resource management" << std::endl;
    std::cout << "- Vectored I/O can improve performance" << std::endl;
    std::cout << "- File metadata provides detailed information" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- File descriptors are fundamental to Linux I/O
- open(), read(), write(), close() are basic operations
- fcntl() controls file descriptor properties
- stat() provides file metadata and permissions
- Vectored I/O (readv/writev) can improve performance
- RAII ensures proper resource cleanup
- Error handling is critical for robust programs
- Performance varies with I/O patterns and buffering
*/
