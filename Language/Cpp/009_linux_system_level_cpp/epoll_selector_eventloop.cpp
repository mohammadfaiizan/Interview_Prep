/*
 * LINUX SYSTEM LEVEL C++ - EPOLL AND EVENT LOOPS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread epoll_selector_eventloop.cpp -o epoll_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl
#define MAX_EVENTS 64

// =============================================================================
// 1. BASIC EPOLL USAGE
// =============================================================================

void demonstrate_basic_epoll() {
    SECTION("Basic Epoll Usage");
    
    // Create epoll instance
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1 failed: " << strerror(errno) << std::endl;
        return;
    }
    
    std::cout << "Created epoll instance: " << epoll_fd << std::endl;
    
    // Add stdin to epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = STDIN_FILENO;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1) {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << std::endl;
        close(epoll_fd);
        return;
    }
    
    std::cout << "Added stdin to epoll. Type something and press Enter (or 'quit' to exit):" << std::endl;
    
    // Event loop
    struct epoll_event events[MAX_EVENTS];
    bool running = true;
    
    while (running) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // 1 second timeout
        
        if (nfds == -1) {
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            break;
        } else if (nfds == 0) {
            std::cout << "Timeout - no events" << std::endl;
            continue;
        }
        
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == STDIN_FILENO) {
                std::string input;
                std::getline(std::cin, input);
                
                if (input == "quit") {
                    running = false;
                } else {
                    std::cout << "You typed: " << input << std::endl;
                }
            }
        }
    }
    
    close(epoll_fd);
    std::cout << "Basic epoll demonstration complete" << std::endl;
}

// =============================================================================
// 2. EPOLL EVENT LOOP CLASS
// =============================================================================

class EventLoop {
private:
    int epoll_fd_;
    bool running_;
    std::unordered_map<int, std::function<void(uint32_t)>> callbacks_;
    
public:
    EventLoop() : epoll_fd_(-1), running_(false) {
        epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd_ == -1) {
            throw std::runtime_error("Failed to create epoll instance");
        }
    }
    
    ~EventLoop() {
        if (epoll_fd_ != -1) {
            close(epoll_fd_);
        }
    }
    
    bool add_fd(int fd, uint32_t events, std::function<void(uint32_t)> callback) {
        struct epoll_event event;
        event.events = events;
        event.data.fd = fd;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1) {
            std::cerr << "Failed to add fd " << fd << ": " << strerror(errno) << std::endl;
            return false;
        }
        
        callbacks_[fd] = callback;
        return true;
    }
    
    bool remove_fd(int fd) {
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            std::cerr << "Failed to remove fd " << fd << ": " << strerror(errno) << std::endl;
            return false;
        }
        
        callbacks_.erase(fd);
        return true;
    }
    
    void run(int timeout_ms = -1) {
        running_ = true;
        struct epoll_event events[MAX_EVENTS];
        
        while (running_) {
            int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
            
            if (nfds == -1) {
                if (errno == EINTR) continue; // Interrupted by signal
                std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
                break;
            }
            
            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                uint32_t event_mask = events[i].events;
                
                auto it = callbacks_.find(fd);
                if (it != callbacks_.end()) {
                    it->second(event_mask);
                }
            }
        }
    }
    
    void stop() {
        running_ = false;
    }
};

void demonstrate_event_loop_class() {
    SECTION("Event Loop Class");
    
    try {
        EventLoop loop;
        
        // Add stdin handler
        loop.add_fd(STDIN_FILENO, EPOLLIN, [&loop](uint32_t events) {
            if (events & EPOLLIN) {
                std::string input;
                std::getline(std::cin, input);
                
                if (input == "quit") {
                    std::cout << "Stopping event loop..." << std::endl;
                    loop.stop();
                } else {
                    std::cout << "Event loop received: " << input << std::endl;
                }
            }
        });
        
        std::cout << "Event loop class demo. Type 'quit' to exit:" << std::endl;
        loop.run(2000); // 2 second timeout
        
    } catch (const std::exception& e) {
        std::cerr << "Event loop error: " << e.what() << std::endl;
    }
}

// =============================================================================
// 3. TCP SERVER WITH EPOLL
// =============================================================================

class EpollTcpServer {
private:
    int server_fd_;
    int epoll_fd_;
    int port_;
    bool running_;
    
    bool set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return false;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
    }
    
    void handle_new_connection() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            }
            return;
        }
        
        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) 
                  << " (fd: " << client_fd << ")" << std::endl;
        
        // Set non-blocking
        if (!set_nonblocking(client_fd)) {
            std::cerr << "Failed to set client non-blocking" << std::endl;
            close(client_fd);
            return;
        }
        
        // Add to epoll
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET; // Edge-triggered
        event.data.fd = client_fd;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) == -1) {
            std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
            close(client_fd);
        }
    }
    
    void handle_client_data(int client_fd) {
        char buffer[1024];
        
        while (true) {
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No more data available
                    break;
                } else {
                    std::cerr << "Read error: " << strerror(errno) << std::endl;
                    close_client(client_fd);
                    return;
                }
            } else if (bytes_read == 0) {
                // Client disconnected
                std::cout << "Client " << client_fd << " disconnected" << std::endl;
                close_client(client_fd);
                return;
            } else {
                buffer[bytes_read] = '\0';
                std::cout << "Client " << client_fd << " sent: " << buffer;
                
                // Echo back
                const char* response = "Echo: ";
                write(client_fd, response, strlen(response));
                write(client_fd, buffer, bytes_read);
            }
        }
    }
    
    void close_client(int client_fd) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr);
        close(client_fd);
    }
    
public:
    explicit EpollTcpServer(int port) : server_fd_(-1), epoll_fd_(-1), port_(port), running_(false) {}
    
    ~EpollTcpServer() {
        stop();
    }
    
    bool start() {
        // Create server socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == -1) {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // Set non-blocking
        if (!set_nonblocking(server_fd_)) {
            std::cerr << "Failed to set server non-blocking" << std::endl;
            return false;
        }
        
        // Bind
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == -1) {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Listen
        if (listen(server_fd_, SOMAXCONN) == -1) {
            std::cerr << "Listen failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Create epoll
        epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd_ == -1) {
            std::cerr << "epoll_create1 failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Add server socket to epoll
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = server_fd_;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &event) == -1) {
            std::cerr << "Failed to add server to epoll: " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "TCP Server started on port " << port_ << std::endl;
        return true;
    }
    
    void run() {
        if (epoll_fd_ == -1) return;
        
        running_ = true;
        struct epoll_event events[MAX_EVENTS];
        
        while (running_) {
            int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000);
            
            if (nfds == -1) {
                if (errno == EINTR) continue;
                std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
                break;
            }
            
            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                uint32_t event_mask = events[i].events;
                
                if (fd == server_fd_) {
                    // New connection
                    handle_new_connection();
                } else {
                    // Client data
                    if (event_mask & EPOLLIN) {
                        handle_client_data(fd);
                    }
                    if (event_mask & (EPOLLHUP | EPOLLERR)) {
                        std::cout << "Client " << fd << " error/hangup" << std::endl;
                        close_client(fd);
                    }
                }
            }
        }
    }
    
    void stop() {
        running_ = false;
        if (epoll_fd_ != -1) {
            close(epoll_fd_);
            epoll_fd_ = -1;
        }
        if (server_fd_ != -1) {
            close(server_fd_);
            server_fd_ = -1;
        }
    }
};

void demonstrate_epoll_server() {
    SECTION("TCP Server with Epoll");
    
    std::cout << "Starting epoll-based TCP server on port 8080" << std::endl;
    std::cout << "Connect with: telnet localhost 8080" << std::endl;
    std::cout << "Server will run for 10 seconds..." << std::endl;
    
    EpollTcpServer server(8080);
    if (server.start()) {
        // Run server in background for demonstration
        std::thread server_thread([&server]() {
            server.run();
        });
        
        // Let it run for a bit
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        server.stop();
        server_thread.join();
    }
    
    std::cout << "Epoll server demonstration complete" << std::endl;
}

int main() {
    std::cout << "LINUX EPOLL AND EVENT LOOPS\n";
    std::cout << "============================\n";
    
    // Note: Some demonstrations require user interaction
    std::cout << "Running automated demonstrations...\n" << std::endl;
    
    demonstrate_event_loop_class();
    demonstrate_epoll_server();
    
    std::cout << "\nTo run interactive demonstrations:" << std::endl;
    std::cout << "1. Uncomment demonstrate_basic_epoll() for stdin demo" << std::endl;
    std::cout << "2. Run the epoll server and connect with telnet" << std::endl;
    
    // Uncomment for interactive demo:
    // demonstrate_basic_epoll();
    
    std::cout << "\nKey Epoll Concepts:" << std::endl;
    std::cout << "- epoll provides scalable I/O event notification" << std::endl;
    std::cout << "- Edge-triggered mode reduces system calls" << std::endl;
    std::cout << "- Non-blocking I/O prevents thread blocking" << std::endl;
    std::cout << "- Event loops enable single-threaded concurrency" << std::endl;
    std::cout << "- Epoll scales to thousands of connections" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Epoll is Linux's scalable I/O multiplexing mechanism
- Edge-triggered mode is more efficient but requires careful handling
- Non-blocking I/O is essential for high-performance servers
- Event loops enable handling many connections in single thread
- Proper error handling is crucial for robust network servers
- RAII principles help manage file descriptors and resources
- Epoll scales much better than select/poll for many connections
- Combines well with thread pools for CPU-intensive work
*/
