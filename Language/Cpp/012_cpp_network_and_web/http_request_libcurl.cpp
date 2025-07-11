/*
 * C++ NETWORK AND WEB - HTTP REQUESTS WITH LIBCURL
 * 
 * This file demonstrates comprehensive HTTP client functionality using libcurl
 * including GET, POST, authentication, headers, and advanced features.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra http_request_libcurl.cpp -lcurl -o http_client
 * 
 * Dependencies: libcurl-dev (Ubuntu/Debian) or libcurl-devel (CentOS/RHEL)
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <functional>

// Mock curl interface for demonstration (in real code, include <curl/curl.h>)
typedef void CURL;
typedef int CURLcode;
typedef int CURLoption;

#define CURLE_OK 0
#define CURLOPT_URL 10002
#define CURLOPT_WRITEFUNCTION 20011
#define CURLOPT_WRITEDATA 10001
#define CURLOPT_HTTPHEADER 10023
#define CURLOPT_POSTFIELDS 10015
#define CURLOPT_CUSTOMREQUEST 10036
#define CURLOPT_USERPWD 10005
#define CURLOPT_FOLLOWLOCATION 52
#define CURLOPT_TIMEOUT 13
#define CURLOPT_SSL_VERIFYPEER 64
#define CURLOPT_VERBOSE 41

struct curl_slist {
    char *data;
    struct curl_slist *next;
};

// Mock curl functions
CURL* curl_easy_init() { return reinterpret_cast<CURL*>(0x12345); }
void curl_easy_cleanup(CURL* curl) { (void)curl; }
CURLcode curl_easy_setopt(CURL* curl, CURLoption option, ...) { (void)curl; (void)option; return CURLE_OK; }
CURLcode curl_easy_perform(CURL* curl) { (void)curl; return CURLE_OK; }
struct curl_slist* curl_slist_append(struct curl_slist* list, const char* string) { (void)list; (void)string; return nullptr; }
void curl_slist_free_all(struct curl_slist* list) { (void)list; }
const char* curl_easy_strerror(CURLcode code) { (void)code; return "Mock error"; }

// =============================================================================
// HTTP CLIENT WRAPPER CLASS
// =============================================================================

class HttpClient {
public:
    struct Response {
        std::string body;
        long status_code = 0;
        std::map<std::string, std::string> headers;
        bool success = false;
        std::string error_message;
    };
    
    struct RequestOptions {
        std::map<std::string, std::string> headers;
        std::string user_agent = "HttpClient/1.0";
        long timeout = 30;
        bool follow_redirects = true;
        bool verify_ssl = true;
        bool verbose = false;
        std::string username;
        std::string password;
    };

private:
    CURL* curl_;
    static std::string response_buffer_;
    static std::string header_buffer_;
    
    // Callback function for writing response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        userp->append(static_cast<char*>(contents), total_size);
        return total_size;
    }
    
    // Callback function for writing header data
    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        userp->append(static_cast<char*>(contents), total_size);
        return total_size;
    }
    
    void SetCommonOptions(const RequestOptions& options) {
        // Set basic options
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_buffer_);
        
        // Set timeout
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, options.timeout);
        
        // Set redirect following
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, options.follow_redirects ? 1L : 0L);
        
        // Set SSL verification
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, options.verify_ssl ? 1L : 0L);
        
        // Set verbose mode
        curl_easy_setopt(curl_, CURLOPT_VERBOSE, options.verbose ? 1L : 0L);
        
        // Set authentication if provided
        if (!options.username.empty()) {
            std::string auth = options.username + ":" + options.password;
            curl_easy_setopt(curl_, CURLOPT_USERPWD, auth.c_str());
        }
    }
    
    struct curl_slist* BuildHeaderList(const std::map<std::string, std::string>& headers) {
        struct curl_slist* header_list = nullptr;
        
        for (const auto& [key, value] : headers) {
            std::string header = key + ": " + value;
            header_list = curl_slist_append(header_list, header.c_str());
        }
        
        return header_list;
    }

public:
    HttpClient() {
        curl_ = curl_easy_init();
        if (!curl_) {
            throw std::runtime_error("Failed to initialize curl");
        }
    }
    
    ~HttpClient() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
    }
    
    // GET request
    Response Get(const std::string& url, const RequestOptions& options = {}) {
        Response response;
        response_buffer_.clear();
        header_buffer_.clear();
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        SetCommonOptions(options);
        
        // Set headers
        struct curl_slist* headers = BuildHeaderList(options.headers);
        if (headers) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        }
        
        // Perform request
        CURLcode res = curl_easy_perform(curl_);
        
        if (res == CURLE_OK) {
            response.success = true;
            response.body = response_buffer_;
            response.status_code = 200; // Mock status
        } else {
            response.success = false;
            response.error_message = curl_easy_strerror(res);
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    // POST request
    Response Post(const std::string& url, const std::string& data, 
                  const RequestOptions& options = {}) {
        Response response;
        response_buffer_.clear();
        header_buffer_.clear();
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        SetCommonOptions(options);
        
        // Set headers
        auto headers_map = options.headers;
        if (headers_map.find("Content-Type") == headers_map.end()) {
            headers_map["Content-Type"] = "application/x-www-form-urlencoded";
        }
        
        struct curl_slist* headers = BuildHeaderList(headers_map);
        if (headers) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        }
        
        // Perform request
        CURLcode res = curl_easy_perform(curl_);
        
        if (res == CURLE_OK) {
            response.success = true;
            response.body = response_buffer_;
            response.status_code = 200; // Mock status
        } else {
            response.success = false;
            response.error_message = curl_easy_strerror(res);
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    // PUT request
    Response Put(const std::string& url, const std::string& data,
                 const RequestOptions& options = {}) {
        Response response;
        response_buffer_.clear();
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        SetCommonOptions(options);
        
        struct curl_slist* headers = BuildHeaderList(options.headers);
        if (headers) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        }
        
        CURLcode res = curl_easy_perform(curl_);
        
        if (res == CURLE_OK) {
            response.success = true;
            response.body = response_buffer_;
            response.status_code = 200;
        } else {
            response.success = false;
            response.error_message = curl_easy_strerror(res);
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    // DELETE request
    Response Delete(const std::string& url, const RequestOptions& options = {}) {
        Response response;
        response_buffer_.clear();
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
        SetCommonOptions(options);
        
        struct curl_slist* headers = BuildHeaderList(options.headers);
        if (headers) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        }
        
        CURLcode res = curl_easy_perform(curl_);
        
        if (res == CURLE_OK) {
            response.success = true;
            response.body = response_buffer_;
            response.status_code = 200;
        } else {
            response.success = false;
            response.error_message = curl_easy_strerror(res);
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    // Download file
    bool DownloadFile(const std::string& url, const std::string& filename,
                      const RequestOptions& options = {}) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, 
                        [](void* ptr, size_t size, size_t nmemb, std::ofstream* stream) -> size_t {
                            stream->write(static_cast<char*>(ptr), size * nmemb);
                            return size * nmemb;
                        });
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &file);
        SetCommonOptions(options);
        
        CURLcode res = curl_easy_perform(curl_);
        file.close();
        
        return res == CURLE_OK;
    }
};

// Static member definitions
std::string HttpClient::response_buffer_;
std::string HttpClient::header_buffer_;

// =============================================================================
// JSON HELPER CLASS
// =============================================================================

class SimpleJson {
private:
    std::map<std::string, std::string> data_;
    
public:
    void Set(const std::string& key, const std::string& value) {
        data_[key] = value;
    }
    
    std::string Get(const std::string& key) const {
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : "";
    }
    
    std::string ToString() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : data_) {
            if (!first) oss << ",";
            oss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        oss << "}";
        return oss.str();
    }
    
    static SimpleJson FromString(const std::string& json_str) {
        // Simple JSON parser for demonstration
        SimpleJson json;
        // In real implementation, use a proper JSON library like nlohmann/json
        return json;
    }
};

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_basic_requests() {
    std::cout << "\n=== Basic HTTP Requests ===" << std::endl;
    
    HttpClient client;
    
    // GET request
    std::cout << "\n1. GET Request:" << std::endl;
    auto response = client.Get("https://httpbin.org/get");
    if (response.success) {
        std::cout << "✓ GET request successful" << std::endl;
        std::cout << "Status: " << response.status_code << std::endl;
        std::cout << "Body preview: " << response.body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cout << "✗ GET request failed: " << response.error_message << std::endl;
    }
    
    // POST request
    std::cout << "\n2. POST Request:" << std::endl;
    std::string post_data = "name=John&email=john@example.com";
    response = client.Post("https://httpbin.org/post", post_data);
    if (response.success) {
        std::cout << "✓ POST request successful" << std::endl;
        std::cout << "Status: " << response.status_code << std::endl;
    } else {
        std::cout << "✗ POST request failed: " << response.error_message << std::endl;
    }
    
    // PUT request
    std::cout << "\n3. PUT Request:" << std::endl;
    std::string put_data = "{\"name\":\"John Updated\",\"email\":\"john.updated@example.com\"}";
    HttpClient::RequestOptions put_options;
    put_options.headers["Content-Type"] = "application/json";
    response = client.Put("https://httpbin.org/put", put_data, put_options);
    if (response.success) {
        std::cout << "✓ PUT request successful" << std::endl;
    } else {
        std::cout << "✗ PUT request failed: " << response.error_message << std::endl;
    }
    
    // DELETE request
    std::cout << "\n4. DELETE Request:" << std::endl;
    response = client.Delete("https://httpbin.org/delete");
    if (response.success) {
        std::cout << "✓ DELETE request successful" << std::endl;
    } else {
        std::cout << "✗ DELETE request failed: " << response.error_message << std::endl;
    }
}

void demonstrate_advanced_features() {
    std::cout << "\n=== Advanced HTTP Features ===" << std::endl;
    
    HttpClient client;
    
    // Custom headers
    std::cout << "\n1. Custom Headers:" << std::endl;
    HttpClient::RequestOptions options;
    options.headers["User-Agent"] = "MyApp/1.0";
    options.headers["Accept"] = "application/json";
    options.headers["Authorization"] = "Bearer your-token-here";
    
    auto response = client.Get("https://httpbin.org/headers", options);
    if (response.success) {
        std::cout << "✓ Request with custom headers successful" << std::endl;
    }
    
    // Authentication
    std::cout << "\n2. Basic Authentication:" << std::endl;
    HttpClient::RequestOptions auth_options;
    auth_options.username = "user";
    auth_options.password = "password";
    
    response = client.Get("https://httpbin.org/basic-auth/user/password", auth_options);
    if (response.success) {
        std::cout << "✓ Basic authentication successful" << std::endl;
    }
    
    // JSON POST
    std::cout << "\n3. JSON POST Request:" << std::endl;
    SimpleJson json_data;
    json_data.Set("name", "John Doe");
    json_data.Set("email", "john@example.com");
    json_data.Set("age", "30");
    
    HttpClient::RequestOptions json_options;
    json_options.headers["Content-Type"] = "application/json";
    json_options.headers["Accept"] = "application/json";
    
    response = client.Post("https://httpbin.org/post", json_data.ToString(), json_options);
    if (response.success) {
        std::cout << "✓ JSON POST request successful" << std::endl;
    }
    
    // File download
    std::cout << "\n4. File Download:" << std::endl;
    bool download_success = client.DownloadFile(
        "https://httpbin.org/json", 
        "downloaded_file.json"
    );
    if (download_success) {
        std::cout << "✓ File downloaded successfully" << std::endl;
    } else {
        std::cout << "✗ File download failed" << std::endl;
    }
}

void demonstrate_error_handling() {
    std::cout << "\n=== Error Handling ===" << std::endl;
    
    HttpClient client;
    
    // Timeout handling
    std::cout << "\n1. Timeout Handling:" << std::endl;
    HttpClient::RequestOptions timeout_options;
    timeout_options.timeout = 1; // 1 second timeout
    
    auto response = client.Get("https://httpbin.org/delay/5", timeout_options);
    if (!response.success) {
        std::cout << "✓ Timeout handled correctly: " << response.error_message << std::endl;
    }
    
    // Invalid URL
    std::cout << "\n2. Invalid URL Handling:" << std::endl;
    response = client.Get("invalid-url");
    if (!response.success) {
        std::cout << "✓ Invalid URL handled: " << response.error_message << std::endl;
    }
    
    // Network error simulation
    std::cout << "\n3. Network Error Handling:" << std::endl;
    response = client.Get("https://nonexistent-domain-12345.com");
    if (!response.success) {
        std::cout << "✓ Network error handled: " << response.error_message << std::endl;
    }
}

void demonstrate_rest_api_client() {
    std::cout << "\n=== REST API Client Example ===" << std::endl;
    
    class RestApiClient {
    private:
        HttpClient http_client_;
        std::string base_url_;
        std::map<std::string, std::string> default_headers_;
        
    public:
        RestApiClient(const std::string& base_url) : base_url_(base_url) {
            default_headers_["Content-Type"] = "application/json";
            default_headers_["Accept"] = "application/json";
        }
        
        void SetAuthToken(const std::string& token) {
            default_headers_["Authorization"] = "Bearer " + token;
        }
        
        HttpClient::Response GetResource(const std::string& endpoint) {
            HttpClient::RequestOptions options;
            options.headers = default_headers_;
            return http_client_.Get(base_url_ + endpoint, options);
        }
        
        HttpClient::Response CreateResource(const std::string& endpoint, const SimpleJson& data) {
            HttpClient::RequestOptions options;
            options.headers = default_headers_;
            return http_client_.Post(base_url_ + endpoint, data.ToString(), options);
        }
        
        HttpClient::Response UpdateResource(const std::string& endpoint, const SimpleJson& data) {
            HttpClient::RequestOptions options;
            options.headers = default_headers_;
            return http_client_.Put(base_url_ + endpoint, data.ToString(), options);
        }
        
        HttpClient::Response DeleteResource(const std::string& endpoint) {
            HttpClient::RequestOptions options;
            options.headers = default_headers_;
            return http_client_.Delete(base_url_ + endpoint, options);
        }
    };
    
    // Usage example
    RestApiClient api_client("https://jsonplaceholder.typicode.com");
    
    std::cout << "\n1. Get all users:" << std::endl;
    auto response = api_client.GetResource("/users");
    if (response.success) {
        std::cout << "✓ Retrieved users list" << std::endl;
    }
    
    std::cout << "\n2. Get specific user:" << std::endl;
    response = api_client.GetResource("/users/1");
    if (response.success) {
        std::cout << "✓ Retrieved user details" << std::endl;
    }
    
    std::cout << "\n3. Create new user:" << std::endl;
    SimpleJson new_user;
    new_user.Set("name", "John Doe");
    new_user.Set("email", "john@example.com");
    response = api_client.CreateResource("/users", new_user);
    if (response.success) {
        std::cout << "✓ Created new user" << std::endl;
    }
}

void demonstrate_async_requests() {
    std::cout << "\n=== Asynchronous Requests (Concept) ===" << std::endl;
    
    std::cout << "\nFor true async HTTP requests, consider:" << std::endl;
    std::cout << "1. std::async with std::future:" << std::endl;
    std::cout << R"(
auto future = std::async(std::launch::async, [&client, url]() {
    return client.Get(url);
});

// Do other work...

auto response = future.get();  // Wait for completion
)" << std::endl;

    std::cout << "\n2. Thread pool for multiple requests:" << std::endl;
    std::cout << R"(
class AsyncHttpClient {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    
public:
    template<typename F>
    auto enqueue(F&& f) -> std::future<typename std::result_of<F()>::type> {
        // Implementation for async task execution
    }
};
)" << std::endl;

    std::cout << "\n3. External libraries for async HTTP:" << std::endl;
    std::cout << "• cpp-httplib (header-only)" << std::endl;
    std::cout << "• Beast (Boost.Beast)" << std::endl;
    std::cout << "• cpprest (Microsoft C++ REST SDK)" << std::endl;
    std::cout << "• libcurl with multi interface" << std::endl;
}

void demonstrate_best_practices() {
    std::cout << "\n=== HTTP Client Best Practices ===" << std::endl;
    
    std::cout << "\n1. Connection Management:" << std::endl;
    std::cout << "• Reuse HttpClient instances for connection pooling" << std::endl;
    std::cout << "• Set appropriate timeouts" << std::endl;
    std::cout << "• Handle redirects properly" << std::endl;
    std::cout << "• Use keep-alive connections" << std::endl;
    
    std::cout << "\n2. Error Handling:" << std::endl;
    std::cout << "• Always check response.success before using data" << std::endl;
    std::cout << "• Implement retry logic with exponential backoff" << std::endl;
    std::cout << "• Log errors for debugging" << std::endl;
    std::cout << "• Handle different HTTP status codes appropriately" << std::endl;
    
    std::cout << "\n3. Security:" << std::endl;
    std::cout << "• Validate SSL certificates in production" << std::endl;
    std::cout << "• Use secure authentication methods" << std::endl;
    std::cout << "• Sanitize user input in URLs and data" << std::endl;
    std::cout << "• Store credentials securely" << std::endl;
    
    std::cout << "\n4. Performance:" << std::endl;
    std::cout << "• Use compression (gzip, deflate)" << std::endl;
    std::cout << "• Implement caching where appropriate" << std::endl;
    std::cout << "• Use async requests for multiple operations" << std::endl;
    std::cout << "• Monitor and limit concurrent connections" << std::endl;
    
    std::cout << "\n5. CMake Integration:" << std::endl;
    std::cout << R"(
# CMakeLists.txt
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        ${CURL_LIBRARIES})
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CURL_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} 
    PRIVATE 
        ${CURL_CFLAGS_OTHER})
)" << std::endl;
}

int main() {
    std::cout << "HTTP REQUESTS WITH LIBCURL\n";
    std::cout << "==========================\n";
    
    demonstrate_basic_requests();
    demonstrate_advanced_features();
    demonstrate_error_handling();
    demonstrate_rest_api_client();
    demonstrate_async_requests();
    demonstrate_best_practices();
    
    std::cout << "\nKey HTTP Client Concepts:" << std::endl;
    std::cout << "• libcurl provides comprehensive HTTP functionality" << std::endl;
    std::cout << "• Proper error handling is essential for robust clients" << std::endl;
    std::cout << "• REST API clients benefit from structured wrapper classes" << std::endl;
    std::cout << "• Asynchronous requests improve application responsiveness" << std::endl;
    std::cout << "• Security and performance considerations are crucial" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- libcurl is the de facto standard for HTTP clients in C++
- Wrapper classes provide cleaner, more maintainable interfaces
- Error handling and timeouts are critical for production use
- JSON integration makes REST API consumption easier
- Asynchronous patterns improve scalability
- Security considerations must be built in from the start
- Connection reuse and pooling optimize performance
- CMake integration simplifies dependency management
*/
