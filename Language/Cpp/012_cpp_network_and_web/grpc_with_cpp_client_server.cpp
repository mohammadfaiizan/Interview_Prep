/*
 * C++ NETWORK AND WEB - GRPC CLIENT AND SERVER
 * 
 * This file demonstrates gRPC implementation in C++ including
 * service definition, server implementation, and client usage.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra grpc_with_cpp_client_server.cpp 
 *              -lgrpc++ -lgrpc -lprotobuf -o grpc_demo
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <map>

// Mock gRPC interface for demonstration
namespace grpc {
    class Status {
    public:
        enum Code { OK = 0, CANCELLED = 1, UNKNOWN = 2, INVALID_ARGUMENT = 3 };
        
        Status() : code_(OK) {}
        Status(Code code, const std::string& msg) : code_(code), error_message_(msg) {}
        
        bool ok() const { return code_ == OK; }
        Code error_code() const { return code_; }
        std::string error_message() const { return error_message_; }
        
        static Status OK_STATUS() { return Status(); }
        static Status CANCELLED_STATUS() { return Status(CANCELLED, "Cancelled"); }
        
    private:
        Code code_;
        std::string error_message_;
    };
    
    class ServerContext {
    public:
        std::string peer() const { return "192.168.1.100:12345"; }
        bool IsCancelled() const { return false; }
    };
    
    class ClientContext {
    public:
        void set_deadline(std::chrono::system_clock::time_point deadline) {
            deadline_ = deadline;
        }
        
    private:
        std::chrono::system_clock::time_point deadline_;
    };
    
    template<typename T>
    class ServerWriter {
    public:
        bool Write(const T& message) {
            std::cout << "Server streaming message" << std::endl;
            return true;
        }
    };
    
    template<typename T>
    class ClientReader {
    public:
        bool Read(T* message) {
            static int count = 0;
            if (count++ < 3) {
                std::cout << "Client reading streamed message " << count << std::endl;
                return true;
            }
            return false;
        }
        
        Status Finish() { return Status::OK_STATUS(); }
    };
    
    class Server {
    public:
        class Builder {
        public:
            Builder& AddListeningPort(const std::string& addr, void* creds) {
                address_ = addr;
                return *this;
            }
            
            Builder& RegisterService(void* service) {
                return *this;
            }
            
            std::unique_ptr<Server> BuildAndStart() {
                std::cout << "gRPC Server starting on " << address_ << std::endl;
                return std::make_unique<Server>();
            }
            
        private:
            std::string address_;
        };
        
        void Wait() {
            std::cout << "Server waiting for requests..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        void Shutdown() {
            std::cout << "Server shutting down" << std::endl;
        }
    };
    
    template<typename T>
    class Service {
    public:
        virtual ~Service() = default;
    };
    
    void* InsecureServerCredentials() { return nullptr; }
    void* InsecureChannelCredentials() { return nullptr; }
    
    class Channel {
    public:
        enum ConnectivityState { IDLE, CONNECTING, READY, TRANSIENT_FAILURE, SHUTDOWN };
        
        ConnectivityState GetState(bool try_to_connect) const {
            return READY;
        }
    };
    
    std::shared_ptr<Channel> CreateChannel(const std::string& target, void* creds) {
        std::cout << "Creating gRPC channel to " << target << std::endl;
        return std::make_shared<Channel>();
    }
}

// Mock protobuf message classes
class UserRequest {
public:
    void set_user_id(int id) { user_id_ = id; }
    int user_id() const { return user_id_; }
    
    void set_name(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }
    
private:
    int user_id_ = 0;
    std::string name_;
};

class UserResponse {
public:
    void set_user_id(int id) { user_id_ = id; }
    int user_id() const { return user_id_; }
    
    void set_name(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }
    
    void set_email(const std::string& email) { email_ = email; }
    const std::string& email() const { return email_; }
    
    void set_status(const std::string& status) { status_ = status; }
    const std::string& status() const { return status_; }
    
private:
    int user_id_ = 0;
    std::string name_;
    std::string email_;
    std::string status_;
};

class ChatMessage {
public:
    void set_user(const std::string& user) { user_ = user; }
    const std::string& user() const { return user_; }
    
    void set_message(const std::string& message) { message_ = message; }
    const std::string& message() const { return message_; }
    
    void set_timestamp(int64_t timestamp) { timestamp_ = timestamp; }
    int64_t timestamp() const { return timestamp_; }
    
private:
    std::string user_;
    std::string message_;
    int64_t timestamp_ = 0;
};

class Empty {
    // Empty message
};

// =============================================================================
// SERVICE DEFINITIONS
// =============================================================================

// User Service - demonstrates unary RPC
class UserService : public grpc::Service<UserService> {
public:
    virtual ~UserService() = default;
    
    // Unary RPC: GetUser
    virtual grpc::Status GetUser(grpc::ServerContext* context,
                                const UserRequest* request,
                                UserResponse* response) {
        std::cout << "GetUser called for user_id: " << request->user_id() << std::endl;
        
        // Simulate database lookup
        if (request->user_id() <= 0) {
            return grpc::Status(grpc::Status::INVALID_ARGUMENT, "Invalid user ID");
        }
        
        response->set_user_id(request->user_id());
        response->set_name("User_" + std::to_string(request->user_id()));
        response->set_email("user" + std::to_string(request->user_id()) + "@example.com");
        response->set_status("active");
        
        return grpc::Status::OK_STATUS();
    }
    
    // Server streaming RPC: GetUserHistory
    virtual grpc::Status GetUserHistory(grpc::ServerContext* context,
                                       const UserRequest* request,
                                       grpc::ServerWriter<UserResponse>* writer) {
        std::cout << "GetUserHistory called for user_id: " << request->user_id() << std::endl;
        
        // Send multiple responses
        for (int i = 1; i <= 5; ++i) {
            UserResponse response;
            response.set_user_id(request->user_id());
            response.set_name("HistoryEntry_" + std::to_string(i));
            response.set_status("historical");
            
            if (!writer->Write(response)) {
                break; // Client disconnected
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        return grpc::Status::OK_STATUS();
    }
};

// Chat Service - demonstrates bidirectional streaming
class ChatService : public grpc::Service<ChatService> {
private:
    std::vector<ChatMessage> chat_history_;
    
public:
    virtual ~ChatService() = default;
    
    // Bidirectional streaming RPC: Chat
    virtual grpc::Status Chat(grpc::ServerContext* context,
                             grpc::ServerReader<ChatMessage>* reader,
                             grpc::ServerWriter<ChatMessage>* writer) {
        std::cout << "Chat session started" << std::endl;
        
        ChatMessage message;
        while (reader->Read(&message)) {
            std::cout << "Received chat message from " << message.user() 
                      << ": " << message.message() << std::endl;
            
            // Store message in history
            chat_history_.push_back(message);
            
            // Echo message to all connected clients (simplified)
            ChatMessage echo;
            echo.set_user("Server");
            echo.set_message("Echo: " + message.message());
            echo.set_timestamp(std::time(nullptr));
            
            writer->Write(echo);
        }
        
        std::cout << "Chat session ended" << std::endl;
        return grpc::Status::OK_STATUS();
    }
    
    // Server streaming RPC: GetChatHistory
    virtual grpc::Status GetChatHistory(grpc::ServerContext* context,
                                       const Empty* request,
                                       grpc::ServerWriter<ChatMessage>* writer) {
        std::cout << "GetChatHistory called" << std::endl;
        
        for (const auto& message : chat_history_) {
            if (!writer->Write(message)) {
                break;
            }
        }
        
        return grpc::Status::OK_STATUS();
    }
};

// =============================================================================
// SERVER IMPLEMENTATION
// =============================================================================

class GrpcServer {
private:
    std::unique_ptr<grpc::Server> server_;
    UserService user_service_;
    ChatService chat_service_;
    
public:
    void Start(const std::string& server_address) {
        grpc::Server::Builder builder;
        
        // Add listening port
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        
        // Register services
        builder.RegisterService(&user_service_);
        builder.RegisterService(&chat_service_);
        
        // Build and start server
        server_ = builder.BuildAndStart();
        
        std::cout << "gRPC Server listening on " << server_address << std::endl;
    }
    
    void Wait() {
        if (server_) {
            server_->Wait();
        }
    }
    
    void Shutdown() {
        if (server_) {
            server_->Shutdown();
        }
    }
};

// =============================================================================
// CLIENT IMPLEMENTATION
// =============================================================================

class UserServiceClient {
private:
    std::shared_ptr<grpc::Channel> channel_;
    
public:
    UserServiceClient(std::shared_ptr<grpc::Channel> channel) 
        : channel_(channel) {}
    
    UserResponse GetUser(int user_id) {
        UserRequest request;
        request.set_user_id(user_id);
        
        UserResponse response;
        grpc::ClientContext context;
        
        // Set timeout
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
        
        // Make RPC call (simplified for demo)
        std::cout << "Calling GetUser RPC for user_id: " << user_id << std::endl;
        
        // Simulate successful response
        response.set_user_id(user_id);
        response.set_name("User_" + std::to_string(user_id));
        response.set_email("user" + std::to_string(user_id) + "@example.com");
        response.set_status("active");
        
        std::cout << "GetUser RPC completed successfully" << std::endl;
        return response;
    }
    
    void GetUserHistory(int user_id) {
        UserRequest request;
        request.set_user_id(user_id);
        
        grpc::ClientContext context;
        
        // Create reader for streaming response (simplified)
        std::cout << "Calling GetUserHistory RPC for user_id: " << user_id << std::endl;
        
        // Simulate streaming responses
        for (int i = 1; i <= 3; ++i) {
            UserResponse response;
            response.set_user_id(user_id);
            response.set_name("HistoryEntry_" + std::to_string(i));
            response.set_status("historical");
            
            std::cout << "Received history entry: " << response.name() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "GetUserHistory RPC completed" << std::endl;
    }
};

class ChatServiceClient {
private:
    std::shared_ptr<grpc::Channel> channel_;
    
public:
    ChatServiceClient(std::shared_ptr<grpc::Channel> channel) 
        : channel_(channel) {}
    
    void StartChat(const std::string& username) {
        grpc::ClientContext context;
        
        std::cout << "Starting chat session for user: " << username << std::endl;
        
        // Simulate bidirectional streaming
        std::vector<std::string> messages = {
            "Hello everyone!",
            "How is everyone doing?",
            "This gRPC chat is working great!"
        };
        
        for (const auto& msg : messages) {
            ChatMessage message;
            message.set_user(username);
            message.set_message(msg);
            message.set_timestamp(std::time(nullptr));
            
            std::cout << "Sending: " << msg << std::endl;
            
            // Simulate server response
            std::cout << "Server echo: Echo: " << msg << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        std::cout << "Chat session completed" << std::endl;
    }
    
    void GetChatHistory() {
        Empty request;
        grpc::ClientContext context;
        
        std::cout << "Requesting chat history..." << std::endl;
        
        // Simulate receiving chat history
        std::vector<std::string> history = {
            "Alice: Hello everyone!",
            "Bob: Hi Alice!",
            "Charlie: Good morning all!"
        };
        
        for (const auto& msg : history) {
            std::cout << "History: " << msg << std::endl;
        }
        
        std::cout << "Chat history retrieval completed" << std::endl;
    }
};

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_server() {
    std::cout << "\n=== gRPC Server Demo ===" << std::endl;
    
    GrpcServer server;
    
    std::thread server_thread([&server]() {
        server.Start("0.0.0.0:50051");
        server.Wait();
    });
    
    // Let server run for a bit
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    server.Shutdown();
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    std::cout << "Server demo completed" << std::endl;
}

void demonstrate_client() {
    std::cout << "\n=== gRPC Client Demo ===" << std::endl;
    
    // Create channel
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    
    // Test User Service
    std::cout << "\n--- User Service Client ---" << std::endl;
    UserServiceClient user_client(channel);
    
    // Test unary RPC
    auto user_response = user_client.GetUser(123);
    std::cout << "User details - ID: " << user_response.user_id() 
              << ", Name: " << user_response.name()
              << ", Email: " << user_response.email() << std::endl;
    
    // Test server streaming RPC
    user_client.GetUserHistory(123);
    
    // Test Chat Service
    std::cout << "\n--- Chat Service Client ---" << std::endl;
    ChatServiceClient chat_client(channel);
    
    // Test bidirectional streaming
    chat_client.StartChat("TestUser");
    
    // Test server streaming
    chat_client.GetChatHistory();
    
    std::cout << "Client demo completed" << std::endl;
}

void demonstrate_error_handling() {
    std::cout << "\n=== gRPC Error Handling ===" << std::endl;
    
    std::cout << "\n1. Status Codes:" << std::endl;
    std::cout << "• OK: Success" << std::endl;
    std::cout << "• CANCELLED: Operation cancelled" << std::endl;
    std::cout << "• UNKNOWN: Unknown error" << std::endl;
    std::cout << "• INVALID_ARGUMENT: Invalid argument" << std::endl;
    std::cout << "• DEADLINE_EXCEEDED: Timeout" << std::endl;
    std::cout << "• NOT_FOUND: Resource not found" << std::endl;
    std::cout << "• PERMISSION_DENIED: Permission denied" << std::endl;
    std::cout << "• UNAVAILABLE: Service unavailable" << std::endl;
    
    std::cout << "\n2. Error Handling Example:" << std::endl;
    std::cout << R"(
grpc::Status status = stub->GetUser(&context, request, &response);
if (!status.ok()) {
    std::cerr << "RPC failed: " << status.error_code() 
              << " - " << status.error_message() << std::endl;
    
    switch (status.error_code()) {
        case grpc::StatusCode::DEADLINE_EXCEEDED:
            // Handle timeout
            break;
        case grpc::StatusCode::UNAVAILABLE:
            // Handle service unavailable
            break;
        default:
            // Handle other errors
            break;
    }
}
)" << std::endl;

    std::cout << "\n3. Client-side Timeouts:" << std::endl;
    std::cout << R"(
grpc::ClientContext context;
context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

grpc::Status status = stub->GetUser(&context, request, &response);
)" << std::endl;
}

void demonstrate_advanced_features() {
    std::cout << "\n=== Advanced gRPC Features ===" << std::endl;
    
    std::cout << "\n1. Interceptors:" << std::endl;
    std::cout << "• Client interceptors for request/response modification" << std::endl;
    std::cout << "• Server interceptors for authentication, logging" << std::endl;
    std::cout << "• Metrics collection and monitoring" << std::endl;
    
    std::cout << "\n2. Load Balancing:" << std::endl;
    std::cout << "• Round-robin load balancing" << std::endl;
    std::cout << "• Pick-first (connection reuse)" << std::endl;
    std::cout << "• Custom load balancing policies" << std::endl;
    
    std::cout << "\n3. Security:" << std::endl;
    std::cout << "• TLS/SSL encryption" << std::endl;
    std::cout << "• Token-based authentication" << std::endl;
    std::cout << "• Mutual TLS (mTLS)" << std::endl;
    std::cout << "• Custom authentication plugins" << std::endl;
    
    std::cout << "\n4. Streaming Patterns:" << std::endl;
    std::cout << "• Server streaming: One request, multiple responses" << std::endl;
    std::cout << "• Client streaming: Multiple requests, one response" << std::endl;
    std::cout << "• Bidirectional streaming: Multiple requests and responses" << std::endl;
    
    std::cout << "\n5. Protocol Buffer Example:" << std::endl;
    std::cout << R"(
// user.proto
syntax = "proto3";

package user;

service UserService {
    rpc GetUser(UserRequest) returns (UserResponse);
    rpc GetUserHistory(UserRequest) returns (stream UserResponse);
}

message UserRequest {
    int32 user_id = 1;
    string name = 2;
}

message UserResponse {
    int32 user_id = 1;
    string name = 2;
    string email = 3;
    string status = 4;
}
)" << std::endl;
}

void demonstrate_cmake_integration() {
    std::cout << "\n=== CMake Integration ===" << std::endl;
    
    std::cout << R"(
# CMakeLists.txt
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Generate protobuf and gRPC files
set(PROTO_FILES user.proto chat.proto)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    # Protobuf generation
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})
    
    # gRPC generation
    grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE})
    
    list(APPEND ALL_PROTO_SRCS ${PROTO_SRCS})
    list(APPEND ALL_PROTO_HDRS ${PROTO_HDRS})
    list(APPEND ALL_GRPC_SRCS ${GRPC_SRCS})
    list(APPEND ALL_GRPC_HDRS ${GRPC_HDRS})
endforeach()

# Create library for generated files
add_library(proto_lib ${ALL_PROTO_SRCS} ${ALL_GRPC_SRCS})
target_link_libraries(proto_lib protobuf::libprotobuf gRPC::grpc++)

# Server executable
add_executable(grpc_server server.cpp)
target_link_libraries(grpc_server proto_lib)

# Client executable
add_executable(grpc_client client.cpp)
target_link_libraries(grpc_client proto_lib)
)" << std::endl;
}

void demonstrate_production_considerations() {
    std::cout << "\n=== Production Considerations ===" << std::endl;
    
    std::cout << "\n1. Performance:" << std::endl;
    std::cout << "• Use connection pooling for clients" << std::endl;
    std::cout << "• Implement proper resource management" << std::endl;
    std::cout << "• Consider message size limits" << std::endl;
    std::cout << "• Monitor and profile RPC performance" << std::endl;
    
    std::cout << "\n2. Reliability:" << std::endl;
    std::cout << "• Implement retry logic with exponential backoff" << std::endl;
    std::cout << "• Use circuit breakers for fault tolerance" << std::endl;
    std::cout << "• Handle network partitions gracefully" << std::endl;
    std::cout << "• Implement health checks" << std::endl;
    
    std::cout << "\n3. Monitoring:" << std::endl;
    std::cout << "• Log all RPC calls with timing information" << std::endl;
    std::cout << "• Collect metrics on success/failure rates" << std::endl;
    std::cout << "• Monitor resource usage (CPU, memory, network)" << std::endl;
    std::cout << "• Set up alerting for anomalies" << std::endl;
    
    std::cout << "\n4. Deployment:" << std::endl;
    std::cout << "• Use service discovery for dynamic environments" << std::endl;
    std::cout << "• Implement graceful shutdown procedures" << std::endl;
    std::cout << "• Version your protocol buffer definitions" << std::endl;
    std::cout << "• Plan for backward compatibility" << std::endl;
}

int main() {
    std::cout << "GRPC WITH C++ CLIENT AND SERVER\n";
    std::cout << "================================\n";
    
    demonstrate_server();
    demonstrate_client();
    demonstrate_error_handling();
    demonstrate_advanced_features();
    demonstrate_cmake_integration();
    demonstrate_production_considerations();
    
    std::cout << "\nKey gRPC Concepts:" << std::endl;
    std::cout << "• gRPC provides high-performance RPC framework" << std::endl;
    std::cout << "• Protocol Buffers enable efficient serialization" << std::endl;
    std::cout << "• Multiple streaming patterns support various use cases" << std::endl;
    std::cout << "• Built-in load balancing and security features" << std::endl;
    std::cout << "• Strong typing and code generation improve reliability" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- gRPC provides a modern, high-performance RPC framework
- Protocol Buffers offer efficient, cross-language serialization
- Streaming RPCs enable real-time and high-throughput applications
- Strong typing and code generation reduce errors
- Built-in features like load balancing and security simplify development
- Excellent performance characteristics for microservices
- Cross-language compatibility enables polyglot architectures
- Production deployment requires careful consideration of reliability and monitoring
*/
