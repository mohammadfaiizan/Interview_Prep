/*
 * C++ NETWORK AND WEB - WEBSOCKETS WITH BOOST.BEAST
 * 
 * This file demonstrates WebSocket server and client implementation
 * using Boost.Beast for real-time communication.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra websockets_boost.cpp -lboost_system -pthread -o websocket_demo
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <queue>
#include <mutex>

// Mock Boost.Beast WebSocket interface
namespace boost {
namespace beast {
    namespace websocket {
        template<typename Stream>
        class stream {
        public:
            stream(Stream&& s) : underlying_stream_(std::move(s)) {}
            
            template<typename Handler>
            void async_accept(Handler&& handler) {
                std::cout << "WebSocket handshake initiated" << std::endl;
                std::thread([handler]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    handler(std::error_code{});
                }).detach();
            }
            
            template<typename Buffer, typename Handler>
            void async_read(Buffer& buffer, Handler&& handler) {
                std::cout << "WebSocket async read initiated" << std::endl;
                std::thread([handler]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    handler(std::error_code{}, 20); // Mock bytes read
                }).detach();
            }
            
            template<typename Buffer, typename Handler>
            void async_write(Buffer&& buffer, Handler&& handler) {
                std::cout << "WebSocket async write initiated" << std::endl;
                std::thread([handler]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    handler(std::error_code{}, 20); // Mock bytes written
                }).detach();
            }
            
            void close() { std::cout << "WebSocket connection closed" << std::endl; }
            
        private:
            Stream underlying_stream_;
        };
    }
    
    class flat_buffer {
    public:
        void clear() { data_.clear(); }
        std::string data() const { return data_; }
        void consume(size_t n) { if (n < data_.size()) data_.erase(0, n); }
    private:
        std::string data_;
    };
}

namespace asio {
    class io_context {
    public:
        void run() { 
            std::cout << "IO context running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
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
                
                void listen() { std::cout << "WebSocket server listening on " 
                                         << endpoint_.address() << ":" << endpoint_.port() << std::endl; }
                
                template<typename Socket, typename Handler>
                void async_accept(Socket& socket, Handler&& handler) {
                    std::cout << "Async accept for WebSocket" << std::endl;
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
                void close() { std::cout << "TCP socket closed" << std::endl; }
            private:
                io_context& context_;
            };
        };
    }
}
}

using namespace boost::asio;
using tcp = ip::tcp;

// =============================================================================
// WEBSOCKET CONNECTION CLASS
// =============================================================================

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection> {
private:
    boost::beast::websocket::stream<tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
    std::string client_id_;
    std::queue<std::string> write_queue_;
    std::mutex write_mutex_;
    bool writing_ = false;
    
public:
    WebSocketConnection(tcp::socket socket) 
        : ws_(std::move(socket)) {
        static int connection_counter = 0;
        client_id_ = "WS_Client_" + std::to_string(++connection_counter);
    }
    
    const std::string& client_id() const { return client_id_; }
    
    void start() {
        std::cout << "Starting WebSocket connection: " << client_id_ << std::endl;
        
        // Accept the WebSocket handshake
        auto self = shared_from_this();
        ws_.async_accept([this, self](std::error_code ec) {
            if (!ec) {
                std::cout << client_id_ << " WebSocket handshake completed" << std::endl;
                read_message();
            } else {
                std::cout << client_id_ << " handshake failed: " << ec.message() << std::endl;
            }
        });
    }
    
    void send_message(const std::string& message) {
        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.push(message);
        
        if (!writing_) {
            writing_ = true;
            write_next_message();
        }
    }
    
    void close() {
        ws_.close();
    }
    
private:
    void read_message() {
        auto self = shared_from_this();
        
        ws_.async_read(buffer_, [this, self](std::error_code ec, std::size_t bytes_read) {
            if (!ec) {
                std::string message = buffer_.data();
                buffer_.clear();
                
                std::cout << client_id_ << " received: " << message << std::endl;
                process_message(message);
                
                read_message(); // Continue reading
            } else {
                std::cout << client_id_ << " read error: " << ec.message() << std::endl;
            }
        });
    }
    
    void process_message(const std::string& message) {
        // Process different message types
        if (message == "ping") {
            send_message("pong");
        } else if (message.find("echo:") == 0) {
            send_message("echo_response:" + message.substr(5));
        } else if (message == "time") {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            send_message("time:" + std::to_string(time_t));
        } else {
            send_message("unknown_command:" + message);
        }
    }
    
    void write_next_message() {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        if (write_queue_.empty()) {
            writing_ = false;
            return;
        }
        
        std::string message = write_queue_.front();
        write_queue_.pop();
        
        auto self = shared_from_this();
        ws_.async_write(boost::asio::buffer(message),
            [this, self](std::error_code ec, std::size_t) {
                if (!ec) {
                    write_next_message();
                } else {
                    std::cout << client_id_ << " write error: " << ec.message() << std::endl;
                    std::lock_guard<std::mutex> lock(write_mutex_);
                    writing_ = false;
                }
            });
    }
};

// =============================================================================
// WEBSOCKET SERVER
// =============================================================================

class WebSocketServer {
private:
    io_context& io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<WebSocketConnection>> connections_;
    std::mutex connections_mutex_;
    
public:
    WebSocketServer(io_context& io_ctx, unsigned short port)
        : io_context_(io_ctx)
        , acceptor_(io_ctx, tcp::endpoint("127.0.0.1", port)) {
        
        acceptor_.listen();
        std::cout << "WebSocket Server listening on port " << port << std::endl;
    }
    
    void start() {
        accept_connections();
    }
    
    void broadcast_message(const std::string& message) {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        std::cout << "Broadcasting to " << connections_.size() << " clients: " << message << std::endl;
        
        for (auto& conn : connections_) {
            conn->send_message(message);
        }
    }
    
    size_t connection_count() const {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        return connections_.size();
    }
    
private:
    void accept_connections() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        
        acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
            if (!ec) {
                auto connection = std::make_shared<WebSocketConnection>(std::move(*socket));
                
                {
                    std::lock_guard<std::mutex> lock(connections_mutex_);
                    connections_.push_back(connection);
                }
                
                connection->start();
                cleanup_connections();
            } else {
                std::cout << "Accept error: " << ec.message() << std::endl;
            }
            
            accept_connections(); // Continue accepting
        });
    }
    
    void cleanup_connections() {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                [](const std::weak_ptr<WebSocketConnection>& conn) {
                    return conn.expired();
                }),
            connections_.end());
    }
};

// =============================================================================
// WEBSOCKET CLIENT
// =============================================================================

class WebSocketClient {
private:
    io_context& io_context_;
    boost::beast::websocket::stream<tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
    std::queue<std::string> write_queue_;
    std::mutex write_mutex_;
    bool writing_ = false;
    bool connected_ = false;
    
public:
    WebSocketClient(io_context& io_ctx) 
        : io_context_(io_ctx), ws_(io_ctx) {}
    
    void connect(const std::string& host, const std::string& port, const std::string& path) {
        std::cout << "Connecting to WebSocket server: " << host << ":" << port << path << std::endl;
        
        // In real implementation, resolve host and connect
        // For demo, simulate successful connection
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            on_connect(std::error_code{});
        }).detach();
    }
    
    void send_message(const std::string& message) {
        if (!connected_) {
            std::cout << "Not connected, cannot send message" << std::endl;
            return;
        }
        
        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.push(message);
        
        if (!writing_) {
            writing_ = true;
            write_next_message();
        }
    }
    
    void close() {
        if (connected_) {
            ws_.close();
            connected_ = false;
        }
    }
    
private:
    void on_connect(std::error_code ec) {
        if (!ec) {
            std::cout << "WebSocket client connected successfully" << std::endl;
            connected_ = true;
            read_message();
        } else {
            std::cout << "WebSocket client connection failed: " << ec.message() << std::endl;
        }
    }
    
    void read_message() {
        ws_.async_read(buffer_, [this](std::error_code ec, std::size_t) {
            if (!ec) {
                std::string message = buffer_.data();
                buffer_.clear();
                
                std::cout << "Client received: " << message << std::endl;
                read_message(); // Continue reading
            } else {
                std::cout << "Client read error: " << ec.message() << std::endl;
                connected_ = false;
            }
        });
    }
    
    void write_next_message() {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        if (write_queue_.empty()) {
            writing_ = false;
            return;
        }
        
        std::string message = write_queue_.front();
        write_queue_.pop();
        
        ws_.async_write(boost::asio::buffer(message),
            [this](std::error_code ec, std::size_t) {
                if (!ec) {
                    write_next_message();
                } else {
                    std::cout << "Client write error: " << ec.message() << std::endl;
                    std::lock_guard<std::mutex> lock(write_mutex_);
                    writing_ = false;
                }
            });
    }
};

// =============================================================================
// CHAT SERVER EXAMPLE
// =============================================================================

class ChatServer {
private:
    WebSocketServer ws_server_;
    std::vector<std::string> chat_history_;
    std::mutex history_mutex_;
    
public:
    ChatServer(io_context& io_ctx, unsigned short port) 
        : ws_server_(io_ctx, port) {}
    
    void start() {
        ws_server_.start();
        std::cout << "Chat server started" << std::endl;
    }
    
    void send_chat_message(const std::string& user, const std::string& message) {
        std::string chat_msg = user + ": " + message;
        
        {
            std::lock_guard<std::mutex> lock(history_mutex_);
            chat_history_.push_back(chat_msg);
            
            // Keep only last 100 messages
            if (chat_history_.size() > 100) {
                chat_history_.erase(chat_history_.begin());
            }
        }
        
        ws_server_.broadcast_message(chat_msg);
    }
    
    std::vector<std::string> get_chat_history() const {
        std::lock_guard<std::mutex> lock(history_mutex_);
        return chat_history_;
    }
};

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_websocket_server() {
    std::cout << "\n=== WebSocket Server Demo ===" << std::endl;
    
    io_context io_ctx;
    WebSocketServer server(io_ctx, 9001);
    
    server.start();
    
    // Simulate some activity
    std::thread([&server]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        server.broadcast_message("Welcome to WebSocket server!");
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        server.broadcast_message("Server announcement: System status OK");
    }).detach();
    
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "WebSocket server demo completed" << std::endl;
}

void demonstrate_websocket_client() {
    std::cout << "\n=== WebSocket Client Demo ===" << std::endl;
    
    io_context io_ctx;
    WebSocketClient client(io_ctx);
    
    client.connect("localhost", "9001", "/");
    
    // Simulate client activity
    std::thread([&client]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client.send_message("ping");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client.send_message("echo:Hello Server!");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client.send_message("time");
    }).detach();
    
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "WebSocket client demo completed" << std::endl;
}

void demonstrate_chat_server() {
    std::cout << "\n=== Chat Server Demo ===" << std::endl;
    
    io_context io_ctx;
    ChatServer chat_server(io_ctx, 9002);
    
    chat_server.start();
    
    // Simulate chat activity
    std::thread([&chat_server]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        chat_server.send_chat_message("Alice", "Hello everyone!");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        chat_server.send_chat_message("Bob", "Hi Alice! How's it going?");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        chat_server.send_chat_message("Alice", "Great! This WebSocket chat is working well.");
    }).detach();
    
    std::thread([&io_ctx]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        io_ctx.stop();
    }).detach();
    
    io_ctx.run();
    std::cout << "Chat server demo completed" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== WebSocket Best Practices ===" << std::endl;
    
    std::cout << "\n1. Connection Management:" << std::endl;
    std::cout << "• Implement heartbeat/ping-pong for connection health" << std::endl;
    std::cout << "• Handle connection drops gracefully" << std::endl;
    std::cout << "• Limit concurrent connections per client" << std::endl;
    std::cout << "• Clean up resources on disconnect" << std::endl;
    
    std::cout << "\n2. Message Handling:" << std::endl;
    std::cout << "• Use message queues for reliable delivery" << std::endl;
    std::cout << "• Implement message acknowledgment if needed" << std::endl;
    std::cout << "• Handle large messages appropriately" << std::endl;
    std::cout << "• Validate all incoming messages" << std::endl;
    
    std::cout << "\n3. Security:" << std::endl;
    std::cout << "• Use WSS (WebSocket Secure) in production" << std::endl;
    std::cout << "• Implement authentication and authorization" << std::endl;
    std::cout << "• Validate origin headers" << std::endl;
    std::cout << "• Rate limit message sending" << std::endl;
    
    std::cout << "\n4. Performance:" << std::endl;
    std::cout << "• Use binary messages for large data" << std::endl;
    std::cout << "• Implement message compression if beneficial" << std::endl;
    std::cout << "• Consider message batching for high frequency updates" << std::endl;
    std::cout << "• Monitor memory usage for long-lived connections" << std::endl;
    
    std::cout << "\n5. JavaScript Client Example:" << std::endl;
    std::cout << R"(
const ws = new WebSocket('ws://localhost:9001');

ws.onopen = function(event) {
    console.log('Connected to WebSocket server');
    ws.send('Hello Server!');
};

ws.onmessage = function(event) {
    console.log('Received:', event.data);
};

ws.onclose = function(event) {
    console.log('Connection closed');
};

ws.onerror = function(error) {
    console.log('WebSocket error:', error);
};
)" << std::endl;
}

int main() {
    std::cout << "WEBSOCKETS WITH BOOST.BEAST\n";
    std::cout << "============================\n";
    
    demonstrate_websocket_server();
    demonstrate_websocket_client();
    demonstrate_chat_server();
    demonstrate_best_practices();
    
    std::cout << "\nKey WebSocket Concepts:" << std::endl;
    std::cout << "• WebSockets enable real-time bidirectional communication" << std::endl;
    std::cout << "• Boost.Beast provides modern C++ WebSocket implementation" << std::endl;
    std::cout << "• Connection management is crucial for scalable servers" << std::endl;
    std::cout << "• Message queuing ensures reliable communication" << std::endl;
    std::cout << "• Security and performance considerations are essential" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- WebSockets provide full-duplex communication over a single TCP connection
- Boost.Beast offers a modern, header-only WebSocket implementation
- Connection lifecycle management is critical for server stability
- Message queuing and thread safety are essential for reliable communication
- Security considerations include authentication, origin validation, and WSS
- Performance optimization involves message batching and compression
- JavaScript integration makes WebSockets ideal for web applications
- Real-time applications like chat, gaming, and live updates benefit greatly
*/
