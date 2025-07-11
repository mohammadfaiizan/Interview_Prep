/*
 * C++ NETWORK AND WEB - ASYNCHRONOUS TCP SERVER WITH BOOST.ASIO
 * 
 * This file demonstrates building high-performance asynchronous TCP servers
 * using Boost.Asio with modern C++ patterns.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra asio_async_tcp_server.cpp -lboost_system -pthread -o tcp_server
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>

// Mock Boost.Asio interface for demonstration
namespace boost {
namespace asio {
    class io_context {
    public:
        void run() { 
            std::cout << "IO context running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        void stop() { std::cout << "IO context stopped" << std::endl; }
    };
    
    namespace ip {
        class tcp {
        public:
            class endpoint {
            public:
                endpoint(const std::string& addr, unsigned short port) 
                    : address_(addr), port_(port) {}
                std::string address() const { return address_; }
                unsigned short port() const { return port_; }
            private:
                std::string address_;
                unsigned short port_;
            };
            
            class acceptor {
            public:
                acceptor(io_context& ctx, const endpoint& ep) 
                    : context_(ctx), endpoint_(ep) {}
                
                void listen() { std::cout << "Listening on " << endpoint_.address() 
                                         << ":" << endpoint_.port() << std::endl; }
                
                template<typename Handler>
                void async_accept(Handler&& handler) {
                    std::cout << "Async accept initiated" << std::endl;
                    // Simulate async operation
                    std::thread([handler]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        handler(std::error_code{});
                    }).detach();
                }
                
            private:
                io_context& context_;
                endpoint endpoint_;
            };
            
            class socket {
            public:
                socket(io_context& ctx) : context_(ctx) {}
                
                template<typename Buffer, typename Handler>
                void async_read_some(Buffer&& buffer, Handler&& handler) {
                    std::cout << "Async read initiated" << std::endl;
                    std::thread([handler]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        handler(std::error_code{}, 13); // Mock bytes read
                    }).detach();
                }
                
                template<typename Buffer, typename Handler>
                void async_write_some(Buffer&& buffer, Handler&& handler) {
                    std::cout << "Async write initiated" << std::endl;
                    std::thread([handler]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        handler(std::error_code{}, 13); // Mock bytes written
                    }).detach();
                }
                
                void close() { std::cout << "Socket closed" << std::endl; }
                
            private:
                io_context& context_;
            };
        };
    }
    
    template<typename T>
    class buffer {
    public:
        buffer(T* data, size_t size) : data_(data), size_(size) {}
        T* data() const { return data_; }
        size_t size() const { return size_; }
    private:
        T* data_;
        size_t size_;
    };
    
    template<typename T>
    buffer<T> make_buffer(T* data, size_t size) {
        return buffer<T>(data, size);
    }
}
}

using namespace boost::asio;
using tcp = ip::tcp;

// =============================================================================
// BASIC ASYNC TCP SERVER
// =============================================================================

class BasicTcpServer {
private:
    io_context& io_context_;
    tcp::acceptor acceptor_;
    
public:
    BasicTcpServer(io_context& io_ctx, unsigned short port)
        : io_context_(io_ctx)
        , acceptor_(io_ctx, tcp::endpoint("127.0.0.1", port)) {
        
        acceptor_.listen();
        std::cout << "Basic TCP Server listening on port " << port << std::endl;
    }
    
    void start_accept() {
        auto new_connection = std::make_shared<tcp::socket>(io_context_);
        
        acceptor_.async_accept(*new_connection,
            [this, new_connection](std::error_code ec) {
                if (!ec) {
                    std::cout << "New connection accepted" << std::endl;
                    handle_connection(new_connection);
                    start_accept(); // Accept next connection
                } else {
                    std::cout << "Accept error: " << ec.message() << std::endl;
                }
            });
    }
    
private:
    void handle_connection(std::shared_ptr<tcp::socket> socket) {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        
        socket->async_read_some(boost::asio::make_buffer(*buffer),
            [this, socket, buffer](std::error_code ec, std::size_t bytes_read) {
                if (!ec) {
                    std::string message(buffer->data(), bytes_read);
                    std::cout << "Received: " << message << std::endl;
                    
                    // Echo the message back
                    std::string response = "Echo: " + message;
                    auto response_buffer = std::make_shared<std::string>(response);
                    
                    socket->async_write_some(boost::asio::make_buffer(*response_buffer),
                        [socket, response_buffer](std::error_code ec, std::size_t) {
                            if (ec) {
                                std::cout << "Write error: " << ec.message() << std::endl;
                            }
                            socket->close();
                        });
                } else {
                    std::cout << "Read error: " << ec.message() << std::endl;
                    socket->close();
                }
            });
    }
};

// =============================================================================
// ADVANCED CONNECTION HANDLER
// =============================================================================

class Connection : public std::enable_shared_from_this<Connection> {
private:
    tcp::socket socket_;
    std::array<char, 1024> buffer_;
    std::string client_id_;
    
public:
    Connection(io_context& io_ctx) : socket_(io_ctx) {
        static int connection_counter = 0;
        client_id_ = "Client_" + std::to_string(++connection_counter);
    }
    
    tcp::socket& socket() { return socket_; }
    const std::string& client_id() const { return client_id_; }
    
    void start() {
        std::cout << "Starting connection for " << client_id_ << std::endl;
        read_message();
    }
    
private:
    void read_message() {
        auto self = shared_from_this();
        
        socket_.async_read_some(boost::asio::make_buffer(buffer_),
            [this, self](std::error_code ec, std::size_t bytes_read) {
                if (!ec) {
                    process_message(std::string(buffer_.data(), bytes_read));
                    read_message(); // Continue reading
                } else {
                    std::cout << client_id_ << " disconnected: " << ec.message() << std::endl;
                }
            });
    }
    
    void process_message(const std::string& message) {
        std::cout << client_id_ << " sent: " << message << std::endl;
        
        // Process different message types
        if (message.find("PING") == 0) {
            send_response("PONG");
        } else if (message.find("TIME") == 0) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            send_response("TIME: " + std::string(std::ctime(&time_t)));
        } else if (message.find("ECHO") == 0) {
            send_response("ECHO: " + message.substr(5));
        } else {
            send_response("Unknown command: " + message);
        }
    }
    
    void send_response(const std::string& response) {
        auto self = shared_from_this();
        auto response_buffer = std::make_shared<std::string>(response + "\n");
        
        socket_.async_write_some(boost::asio::make_buffer(*response_buffer),
            [this, self, response_buffer](std::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << client_id_ << " write error: " << ec.message() << std::endl;
                }
            });
    }
};

// =============================================================================
// ADVANCED TCP SERVER WITH CONNECTION MANAGEMENT
// =============================================================================

class AdvancedTcpServer {
private:
    io_context& io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Connection>> connections_;
    
public:
    AdvancedTcpServer(io_context& io_ctx, unsigned short port)
        : io_context_(io_ctx)
        , acceptor_(io_ctx, tcp::endpoint("127.0.0.1", port)) {
        
        acceptor_.listen();
        std::cout << "Advanced TCP Server listening on port " << port << std::endl;
    }
    
    void start() {
        accept_connections();
    }
    
    void broadcast_message(const std::string& message) {
        std::cout << "Broadcasting: " << message << std::endl;
        // In real implementation, send to all active connections
    }
    
    size_t connection_count() const {
        return connections_.size();
    }
    
private:
    void accept_connections() {
        auto new_connection = std::make_shared<Connection>(io_context_);
        
        acceptor_.async_accept(new_connection->socket(),
            [this, new_connection](std::error_code ec) {
                if (!ec) {
                    std::cout << "New connection accepted: " << new_connection->client_id() << std::endl;
                    connections_.push_back(new_connection);
                    new_connection->start();
                    
                    // Clean up disconnected connections
                    cleanup_connections();
                } else {
                    std::cout << "Accept error: " << ec.message() << std::endl;
                }
                
                accept_connections(); // Continue accepting
            });
    }
    
    void cleanup_connections() {
        // Remove disconnected connections
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [](const std::weak_ptr<Connection>& conn) {
                    return conn.expired();
                }),
            connections_.end());
    }
};

// =============================================================================
// HTTP-LIKE PROTOCOL SERVER
// =============================================================================

class HttpLikeServer {
private:
    io_context& io_context_;
    tcp::acceptor acceptor_;
    
public:
    HttpLikeServer(io_context& io_ctx, unsigned short port)
        : io_context_(io_ctx)
        , acceptor_(io_ctx, tcp::endpoint("127.0.0.1", port)) {
        
        acceptor_.listen();
        std::cout << "HTTP-like Server listening on port " << port << std::endl;
    }
    
    void start() {
        accept_connections();
    }
    
private:
    void accept_connections() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        
        acceptor_.async_accept(*socket,
            [this, socket](std::error_code ec) {
                if (!ec) {
                    handle_http_request(socket);
                }
                accept_connections();
            });
    }
    
    void handle_http_request(std::shared_ptr<tcp::socket> socket) {
        auto buffer = std::make_shared<std::array<char, 4096>>();
        
        socket->async_read_some(boost::asio::make_buffer(*buffer),
            [this, socket, buffer](std::error_code ec, std::size_t bytes_read) {
                if (!ec) {
                    std::string request(buffer->data(), bytes_read);
                    std::string response = process_http_request(request);
                    send_http_response(socket, response);
                } else {
                    socket->close();
                }
            });
    }
    
    std::string process_http_request(const std::string& request) {
        std::cout << "HTTP Request received:\n" << request << std::endl;
        
        // Parse request line
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        std::string body;
        if (path == "/") {
            body = "<html><body><h1>Welcome to C++ HTTP Server</h1></body></html>";
        } else if (path == "/api/status") {
            body = R"({"status":"ok","server":"cpp-asio","timestamp":")" + 
                   std::to_string(std::time(nullptr)) + "\"}";
        } else {
            body = "<html><body><h1>404 Not Found</h1></body></html>";
        }
        
        return body;
    }
    
    void send_http_response(std::shared_ptr<tcp::socket> socket, const std::string& body) {
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.length()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        
        auto response_buffer = std::make_shared<std::string>(response);
        
        socket->async_write_some(boost::asio::make_buffer(*response_buffer),
            [socket, response_buffer](std::error_code ec, std::size_t) {
                socket->close();
            });
    }
};

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_basic_server() {
    std::cout << "\n=== Basic Async TCP Server ===" << std::endl;
    
    io_context io_ctx;
    BasicTcpServer server(io_ctx, 8080);
    
    server.start_accept();
    
    // Run for a short time in demo
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "Basic server demo completed" << std::endl;
}

void demonstrate_advanced_server() {
    std::cout << "\n=== Advanced TCP Server ===" << std::endl;
    
    io_context io_ctx;
    AdvancedTcpServer server(io_ctx, 8081);
    
    server.start();
    
    // Simulate some activity
    std::thread([&server]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        server.broadcast_message("Server announcement!");
    }).detach();
    
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "Advanced server demo completed" << std::endl;
}

void demonstrate_http_server() {
    std::cout << "\n=== HTTP-like Server ===" << std::endl;
    
    io_context io_ctx;
    HttpLikeServer server(io_ctx, 8082);
    
    server.start();
    
    std::cout << "HTTP server would be accessible at http://localhost:8082" << std::endl;
    
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "HTTP server demo completed" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== Async Server Best Practices ===" << std::endl;
    
    std::cout << "\n1. Connection Management:" << std::endl;
    std::cout << "• Use shared_ptr for connection objects" << std::endl;
    std::cout << "• Implement proper cleanup for disconnected clients" << std::endl;
    std::cout << "• Set connection limits to prevent resource exhaustion" << std::endl;
    std::cout << "• Use connection pools for database access" << std::endl;
    
    std::cout << "\n2. Error Handling:" << std::endl;
    std::cout << "• Always check error codes in async callbacks" << std::endl;
    std::cout << "• Implement graceful degradation for network errors" << std::endl;
    std::cout << "• Log errors with appropriate detail levels" << std::endl;
    std::cout << "• Use timeouts to prevent hanging connections" << std::endl;
    
    std::cout << "\n3. Performance Optimization:" << std::endl;
    std::cout << "• Use multiple io_context instances for CPU-bound work" << std::endl;
    std::cout << "• Implement buffer pooling for high-throughput scenarios" << std::endl;
    std::cout << "• Consider strand for thread-safe operations" << std::endl;
    std::cout << "• Profile and monitor connection patterns" << std::endl;
    
    std::cout << "\n4. Security Considerations:" << std::endl;
    std::cout << "• Validate all input data" << std::endl;
    std::cout << "• Implement rate limiting" << std::endl;
    std::cout << "• Use SSL/TLS for encrypted communication" << std::endl;
    std::cout << "• Monitor for suspicious connection patterns" << std::endl;
    
    std::cout << "\n5. CMake Integration:" << std::endl;
    std::cout << R"(
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Boost::system
        Threads::Threads)
)" << std::endl;
}

int main() {
    std::cout << "ASYNCHRONOUS TCP SERVER WITH BOOST.ASIO\n";
    std::cout << "=======================================\n";
    
    demonstrate_basic_server();
    demonstrate_advanced_server();
    demonstrate_http_server();
    demonstrate_best_practices();
    
    std::cout << "\nKey Async Server Concepts:" << std::endl;
    std::cout << "• Asynchronous I/O enables high concurrency" << std::endl;
    std::cout << "• Proper connection management prevents resource leaks" << std::endl;
    std::cout << "• Error handling is crucial for production servers" << std::endl;
    std::cout << "• Boost.Asio provides powerful networking abstractions" << std::endl;
    std::cout << "• Modern C++ patterns improve code maintainability" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Boost.Asio enables high-performance asynchronous networking
- shared_ptr and enable_shared_from_this manage connection lifetimes
- Async callbacks require careful error handling
- Connection management is critical for server stability
- Modern C++ features improve code safety and readability
- Proper resource management prevents memory leaks
- Scalability requires thoughtful architecture design
- Security considerations must be built in from the start
*/
