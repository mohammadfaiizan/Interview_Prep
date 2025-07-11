/*
 * LINUX SYSTEM LEVEL C++ - TCP/UDP SOCKET PROGRAMMING
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread sockets_tcp_udp.cpp -o sockets
 */

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. TCP SERVER IMPLEMENTATION
// =============================================================================

class TCPServer {
private:
    int server_fd_;
    int port_;
    
public:
    explicit TCPServer(int port) : server_fd_(-1), port_(port) {}
    
    ~TCPServer() {
        if (server_fd_ != -1) {
            close(server_fd_);
        }
    }
    
    bool start() {
        // Create socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == -1) {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "Setsockopt failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Bind to address
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == -1) {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Listen for connections
        if (listen(server_fd_, 5) == -1) {
            std::cerr << "Listen failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "TCP Server listening on port " << port_ << std::endl;
        return true;
    }
    
    void run() {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd == -1) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                continue;
            }
            
            std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << std::endl;
            
            // Handle client in separate thread
            std::thread client_thread(&TCPServer::handle_client, this, client_fd);
            client_thread.detach();
        }
    }
    
private:
    void handle_client(int client_fd) {
        char buffer[1024];
        
        while (true) {
            ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    std::cout << "Client disconnected" << std::endl;
                } else {
                    std::cerr << "Recv failed: " << strerror(errno) << std::endl;
                }
                break;
            }
            
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            
            // Echo back to client
            std::string response = "Echo: " + std::string(buffer);
            send(client_fd, response.c_str(), response.length(), 0);
        }
        
        close(client_fd);
    }
};

// =============================================================================
// 2. TCP CLIENT IMPLEMENTATION
// =============================================================================

class TCPClient {
private:
    int socket_fd_;
    
public:
    TCPClient() : socket_fd_(-1) {}
    
    ~TCPClient() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
        }
    }
    
    bool connect_to_server(const std::string& host, int port) {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << host << std::endl;
            return false;
        }
        
        if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "Connection failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "Connected to " << host << ":" << port << std::endl;
        return true;
    }
    
    bool send_message(const std::string& message) {
        ssize_t bytes_sent = send(socket_fd_, message.c_str(), message.length(), 0);
        if (bytes_sent == -1) {
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    
    std::string receive_message() {
        char buffer[1024];
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            return "";
        }
        buffer[bytes_received] = '\0';
        return std::string(buffer);
    }
};

// =============================================================================
// 3. UDP SERVER IMPLEMENTATION
// =============================================================================

class UDPServer {
private:
    int socket_fd_;
    int port_;
    
public:
    explicit UDPServer(int port) : socket_fd_(-1), port_(port) {}
    
    ~UDPServer() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
        }
    }
    
    bool start() {
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ == -1) {
            std::cerr << "UDP socket creation failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);
        
        if (bind(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "UDP bind failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        std::cout << "UDP Server listening on port " << port_ << std::endl;
        return true;
    }
    
    void run() {
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        while (true) {
            ssize_t bytes_received = recvfrom(socket_fd_, buffer, sizeof(buffer) - 1, 0,
                                            (struct sockaddr*)&client_addr, &client_len);
            if (bytes_received == -1) {
                std::cerr << "Recvfrom failed: " << strerror(errno) << std::endl;
                continue;
            }
            
            buffer[bytes_received] = '\0';
            std::cout << "UDP received from " << inet_ntoa(client_addr.sin_addr) 
                      << ": " << buffer << std::endl;
            
            // Echo back
            std::string response = "UDP Echo: " + std::string(buffer);
            sendto(socket_fd_, response.c_str(), response.length(), 0,
                   (struct sockaddr*)&client_addr, client_len);
        }
    }
};

// =============================================================================
// 4. UDP CLIENT IMPLEMENTATION
// =============================================================================

class UDPClient {
private:
    int socket_fd_;
    struct sockaddr_in server_addr_;
    
public:
    UDPClient() : socket_fd_(-1) {}
    
    ~UDPClient() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
        }
    }
    
    bool setup(const std::string& host, int port) {
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ == -1) {
            std::cerr << "UDP socket creation failed: " << strerror(errno) << std::endl;
            return false;
        }
        
        server_addr_.sin_family = AF_INET;
        server_addr_.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr_.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << host << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool send_message(const std::string& message) {
        ssize_t bytes_sent = sendto(socket_fd_, message.c_str(), message.length(), 0,
                                   (struct sockaddr*)&server_addr_, sizeof(server_addr_));
        if (bytes_sent == -1) {
            std::cerr << "Sendto failed: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    
    std::string receive_message() {
        char buffer[1024];
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        ssize_t bytes_received = recvfrom(socket_fd_, buffer, sizeof(buffer) - 1, 0,
                                        (struct sockaddr*)&from_addr, &from_len);
        if (bytes_received <= 0) {
            return "";
        }
        buffer[bytes_received] = '\0';
        return std::string(buffer);
    }
};

// =============================================================================
// 5. DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_tcp_communication() {
    SECTION("TCP Communication Demonstration");
    
    const int port = 8080;
    
    // Start server in background thread
    std::thread server_thread([]() {
        TCPServer server(8080);
        if (server.start()) {
            // Run for a short time
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Create client and connect
    TCPClient client;
    if (client.connect_to_server("127.0.0.1", port)) {
        // Send messages
        client.send_message("Hello TCP Server!");
        std::string response = client.receive_message();
        std::cout << "Client received: " << response << std::endl;
        
        client.send_message("How are you?");
        response = client.receive_message();
        std::cout << "Client received: " << response << std::endl;
    }
    
    server_thread.join();
}

void demonstrate_udp_communication() {
    SECTION("UDP Communication Demonstration");
    
    const int port = 8081;
    
    // Start UDP server in background thread
    std::thread server_thread([]() {
        UDPServer server(8081);
        if (server.start()) {
            // Run for a short time
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Create UDP client
    UDPClient client;
    if (client.setup("127.0.0.1", port)) {
        // Send messages
        client.send_message("Hello UDP Server!");
        std::string response = client.receive_message();
        std::cout << "UDP Client received: " << response << std::endl;
        
        client.send_message("UDP is connectionless!");
        response = client.receive_message();
        std::cout << "UDP Client received: " << response << std::endl;
    }
    
    server_thread.join();
}

// =============================================================================
// 6. SOCKET OPTIONS AND CONFIGURATION
// =============================================================================

void demonstrate_socket_options() {
    SECTION("Socket Options and Configuration");
    
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }
    
    // SO_REUSEADDR - Allow reuse of local addresses
    int reuse = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0) {
        std::cout << "SO_REUSEADDR set successfully" << std::endl;
    }
    
    // SO_KEEPALIVE - Enable keep-alive packets
    int keepalive = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == 0) {
        std::cout << "SO_KEEPALIVE set successfully" << std::endl;
    }
    
    // SO_RCVBUF - Set receive buffer size
    int rcvbuf = 65536;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) == 0) {
        std::cout << "Receive buffer size set to " << rcvbuf << " bytes" << std::endl;
    }
    
    // SO_SNDBUF - Set send buffer size
    int sndbuf = 65536;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) == 0) {
        std::cout << "Send buffer size set to " << sndbuf << " bytes" << std::endl;
    }
    
    // Get socket options
    socklen_t optlen;
    
    // Get current receive buffer size
    optlen = sizeof(rcvbuf);
    if (getsockopt(tcp_socket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen) == 0) {
        std::cout << "Current receive buffer size: " << rcvbuf << " bytes" << std::endl;
    }
    
    // Get socket type
    int socktype;
    optlen = sizeof(socktype);
    if (getsockopt(tcp_socket, SOL_SOCKET, SO_TYPE, &socktype, &optlen) == 0) {
        std::cout << "Socket type: " << (socktype == SOCK_STREAM ? "SOCK_STREAM" : "SOCK_DGRAM") << std::endl;
    }
    
    close(tcp_socket);
}

// =============================================================================
// 7. NON-BLOCKING SOCKETS
// =============================================================================

void demonstrate_nonblocking_sockets() {
    SECTION("Non-blocking Sockets");
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }
    
    // Set non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking mode" << std::endl;
        close(server_fd);
        return;
    }
    
    std::cout << "Socket set to non-blocking mode" << std::endl;
    
    // Bind and listen
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8082);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }
    
    if (listen(server_fd, 5) == -1) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }
    
    std::cout << "Non-blocking server listening on port 8082" << std::endl;
    
    // Non-blocking accept loop
    for (int i = 0; i < 10; ++i) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "No pending connections (non-blocking)" << std::endl;
            } else {
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
        } else {
            std::cout << "Client connected!" << std::endl;
            close(client_fd);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    close(server_fd);
}

// =============================================================================
// 8. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("TCP vs UDP Performance Comparison");
    
    const int iterations = 1000;
    const std::string test_message = "Performance test message";
    
    std::cout << "Note: This is a simplified comparison. Real performance depends on many factors." << std::endl;
    std::cout << "TCP provides reliability, UDP provides speed." << std::endl;
    std::cout << "TCP: Connection-oriented, reliable, ordered delivery" << std::endl;
    std::cout << "UDP: Connectionless, unreliable, fast" << std::endl;
    
    // TCP characteristics
    std::cout << "\nTCP Characteristics:" << std::endl;
    std::cout << "- Connection establishment overhead" << std::endl;
    std::cout << "- Acknowledgment and retransmission" << std::endl;
    std::cout << "- Flow control and congestion control" << std::endl;
    std::cout << "- Ordered delivery guarantee" << std::endl;
    
    // UDP characteristics  
    std::cout << "\nUDP Characteristics:" << std::endl;
    std::cout << "- No connection establishment" << std::endl;
    std::cout << "- No acknowledgment or retransmission" << std::endl;
    std::cout << "- No flow control" << std::endl;
    std::cout << "- Best-effort delivery" << std::endl;
}

int main() {
    std::cout << "LINUX TCP/UDP SOCKET PROGRAMMING\n";
    std::cout << "=================================\n";
    
    demonstrate_socket_options();
    demonstrate_nonblocking_sockets();
    performance_comparison();
    
    std::cout << "\nTo test communication:" << std::endl;
    std::cout << "1. Uncomment demonstrate_tcp_communication() for TCP test" << std::endl;
    std::cout << "2. Uncomment demonstrate_udp_communication() for UDP test" << std::endl;
    std::cout << "3. Run in separate terminals for full client-server demo" << std::endl;
    
    // Uncomment these for full demonstrations:
    // demonstrate_tcp_communication();
    // demonstrate_udp_communication();
    
    std::cout << "\nKey Socket Concepts:" << std::endl;
    std::cout << "- TCP: Reliable, connection-oriented, stream-based" << std::endl;
    std::cout << "- UDP: Unreliable, connectionless, message-based" << std::endl;
    std::cout << "- Socket options control behavior and performance" << std::endl;
    std::cout << "- Non-blocking sockets enable asynchronous I/O" << std::endl;
    std::cout << "- Always handle errors and clean up resources" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- TCP provides reliable, ordered, connection-oriented communication
- UDP provides fast, connectionless, best-effort communication
- Socket options control buffer sizes, timeouts, and behavior
- Non-blocking sockets enable event-driven programming
- Error handling is crucial for network programming
- RAII principles apply to socket management
- Performance depends on use case and network conditions
- Choose protocol based on reliability vs speed requirements
*/
