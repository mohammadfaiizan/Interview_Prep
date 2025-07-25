/*
 * ADVANCED PROJECTS - EVENT LOOP WITH EPOLL
 * 
 * High-performance event loop implementation using epoll for scalable I/O operations.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread event_loop_epoll.cpp -o event_loop
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>

// Mock epoll for demonstration (in real Linux implementation, use sys/epoll.h)
#ifdef __linux__
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#else
// Mock epoll structures and functions for non-Linux platforms
struct epoll_event {
    uint32_t events;
    union {
        void* ptr;
        int fd;
        uint32_t u32;
        uint64_t u64;
    } data;
};

#define EPOLLIN 0x001
#define EPOLLOUT 0x004
#define EPOLLERR 0x008
#define EPOLLHUP 0x010
#define EPOLLET 0x80000000

int epoll_create1(int flags) { return 1; /* Mock fd */ }
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) { return 0; }
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0; // Mock - no events
}
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3
#endif

// =============================================================================
// EVENT TYPES AND CALLBACKS
// =============================================================================

enum class EventType {
    READ, WRITE, ERROR, TIMEOUT, SIGNAL
};

struct Event {
    int fd;
    EventType type;
    std::chrono::steady_clock::time_point timestamp;
    
    Event(int f, EventType t) : fd(f), type(t), timestamp(std::chrono::steady_clock::now()) {}
};

using EventCallback = std::function<void(const Event&)>;
using TimerCallback = std::function<void()>;

struct Timer {
    std::chrono::steady_clock::time_point expiry;
    std::chrono::milliseconds interval;
    TimerCallback callback;
    bool repeating;
    uint64_t id;
    
    Timer(std::chrono::milliseconds delay, TimerCallback cb, bool repeat = false, uint64_t timer_id = 0)
        : expiry(std::chrono::steady_clock::now() + delay), interval(delay), 
          callback(std::move(cb)), repeating(repeat), id(timer_id) {}
};

// =============================================================================
// FILE DESCRIPTOR MANAGEMENT
// =============================================================================

class FdHandler {
public:
    int fd;
    EventCallback read_callback;
    EventCallback write_callback;
    EventCallback error_callback;
    uint32_t events = 0;
    bool active = true;
    
    FdHandler(int f) : fd(f) {}
    
    void set_read_callback(EventCallback callback) {
        read_callback = std::move(callback);
        events |= EPOLLIN;
    }
    
    void set_write_callback(EventCallback callback) {
        write_callback = std::move(callback);
        events |= EPOLLOUT;
    }
    
    void set_error_callback(EventCallback callback) {
        error_callback = std::move(callback);
        events |= EPOLLERR | EPOLLHUP;
    }
    
    void handle_event(const Event& event) {
        switch (event.type) {
            case EventType::READ:
                if (read_callback) read_callback(event);
                break;
            case EventType::WRITE:
                if (write_callback) write_callback(event);
                break;
            case EventType::ERROR:
                if (error_callback) error_callback(event);
                break;
            default:
                break;
        }
    }
};

// =============================================================================
// HIGH-PERFORMANCE EVENT LOOP
// =============================================================================

class EventLoop {
private:
    int epoll_fd_;
    std::unordered_map<int, std::shared_ptr<FdHandler>> handlers_;
    std::priority_queue<Timer, std::vector<Timer>, std::function<bool(const Timer&, const Timer&)>> timers_;
    
    std::atomic<bool> running_;
    std::mutex handlers_mutex_;
    std::mutex timers_mutex_;
    
    // Statistics
    std::atomic<uint64_t> events_processed_;
    std::atomic<uint64_t> timers_fired_;
    uint64_t next_timer_id_;
    
    // Thread pool for callbacks
    std::vector<std::thread> worker_threads_;
    std::queue<std::function<void()>> task_queue_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;
    
public:
    EventLoop(size_t num_workers = 4) 
        : epoll_fd_(-1), running_(false), events_processed_(0), 
          timers_fired_(0), next_timer_id_(1),
          timers_([](const Timer& a, const Timer& b) { return a.expiry > b.expiry; }) {
        
        // Create epoll instance
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1) {
            throw std::runtime_error("Failed to create epoll instance");
        }
        
        std::cout << "Event loop created with epoll fd: " << epoll_fd_ << std::endl;
        
        // Start worker threads
        for (size_t i = 0; i < num_workers; ++i) {
            worker_threads_.emplace_back([this] { worker_thread(); });
        }
        
        std::cout << "Started " << num_workers << " worker threads" << std::endl;
    }
    
    ~EventLoop() {
        stop();
        
        // Stop worker threads
        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            running_ = false;
        }
        task_cv_.notify_all();
        
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        if (epoll_fd_ != -1) {
            close(epoll_fd_);
        }
        
        std::cout << "Event loop destroyed" << std::endl;
    }
    
    // File descriptor management
    bool add_fd(int fd, std::shared_ptr<FdHandler> handler) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        struct epoll_event event;
        event.events = handler->events | EPOLLET; // Edge-triggered
        event.data.fd = fd;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1) {
            std::cerr << "Failed to add fd " << fd << " to epoll" << std::endl;
            return false;
        }
        
        handlers_[fd] = handler;
        std::cout << "Added fd " << fd << " to event loop" << std::endl;
        return true;
    }
    
    bool modify_fd(int fd, uint32_t events) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(fd);
        if (it == handlers_.end()) {
            return false;
        }
        
        struct epoll_event event;
        event.events = events | EPOLLET;
        event.data.fd = fd;
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == -1) {
            std::cerr << "Failed to modify fd " << fd << " in epoll" << std::endl;
            return false;
        }
        
        it->second->events = events;
        std::cout << "Modified fd " << fd << " events" << std::endl;
        return true;
    }
    
    bool remove_fd(int fd) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            std::cerr << "Failed to remove fd " << fd << " from epoll" << std::endl;
            return false;
        }
        
        handlers_.erase(fd);
        std::cout << "Removed fd " << fd << " from event loop" << std::endl;
        return true;
    }
    
    // Timer management
    uint64_t add_timer(std::chrono::milliseconds delay, TimerCallback callback, bool repeating = false) {
        std::lock_guard<std::mutex> lock(timers_mutex_);
        
        uint64_t timer_id = next_timer_id_++;
        timers_.emplace(delay, std::move(callback), repeating, timer_id);
        
        std::cout << "Added timer " << timer_id << " with delay " << delay.count() << "ms" << std::endl;
        return timer_id;
    }
    
    // Main event loop
    void run() {
        std::cout << "Starting event loop..." << std::endl;
        running_ = true;
        
        const int max_events = 64;
        struct epoll_event events[max_events];
        
        while (running_) {
            // Calculate timeout for next timer
            int timeout = calculate_timeout();
            
            // Wait for events
            int num_events = epoll_wait(epoll_fd_, events, max_events, timeout);
            
            if (num_events == -1) {
                if (errno == EINTR) continue; // Interrupted by signal
                std::cerr << "epoll_wait failed" << std::endl;
                break;
            }
            
            // Process epoll events
            for (int i = 0; i < num_events; ++i) {
                process_epoll_event(events[i]);
            }
            
            // Process timers
            process_timers();
        }
        
        std::cout << "Event loop stopped" << std::endl;
        print_statistics();
    }
    
    void stop() {
        std::cout << "Stopping event loop..." << std::endl;
        running_ = false;
    }
    
    // Async task execution
    void post_task(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            task_queue_.push(std::move(task));
        }
        task_cv_.notify_one();
    }
    
    // Statistics
    void print_statistics() const {
        std::cout << "\n=== Event Loop Statistics ===" << std::endl;
        std::cout << "Events processed: " << events_processed_.load() << std::endl;
        std::cout << "Timers fired: " << timers_fired_.load() << std::endl;
        std::cout << "Active handlers: " << handlers_.size() << std::endl;
        std::cout << "Active timers: " << timers_.size() << std::endl;
        std::cout << "=============================" << std::endl;
    }
    
private:
    void process_epoll_event(const struct epoll_event& epoll_event) {
        int fd = epoll_event.data.fd;
        uint32_t events = epoll_event.events;
        
        std::shared_ptr<FdHandler> handler;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            auto it = handlers_.find(fd);
            if (it == handlers_.end()) {
                return; // Handler was removed
            }
            handler = it->second;
        }
        
        // Dispatch events to worker threads
        if (events & EPOLLIN) {
            post_task([handler, fd] {
                Event event(fd, EventType::READ);
                handler->handle_event(event);
            });
        }
        
        if (events & EPOLLOUT) {
            post_task([handler, fd] {
                Event event(fd, EventType::WRITE);
                handler->handle_event(event);
            });
        }
        
        if (events & (EPOLLERR | EPOLLHUP)) {
            post_task([handler, fd] {
                Event event(fd, EventType::ERROR);
                handler->handle_event(event);
            });
        }
        
        ++events_processed_;
    }
    
    void process_timers() {
        std::lock_guard<std::mutex> lock(timers_mutex_);
        
        auto now = std::chrono::steady_clock::now();
        std::vector<Timer> expired_timers;
        
        while (!timers_.empty() && timers_.top().expiry <= now) {
            expired_timers.push_back(timers_.top());
            timers_.pop();
        }
        
        for (auto& timer : expired_timers) {
            // Execute timer callback in worker thread
            post_task([callback = timer.callback] {
                callback();
            });
            
            ++timers_fired_;
            
            // Re-add repeating timers
            if (timer.repeating) {
                timer.expiry = now + timer.interval;
                timers_.push(timer);
            }
        }
    }
    
    int calculate_timeout() {
        std::lock_guard<std::mutex> lock(timers_mutex_);
        
        if (timers_.empty()) {
            return 1000; // 1 second default timeout
        }
        
        auto now = std::chrono::steady_clock::now();
        auto next_expiry = timers_.top().expiry;
        
        if (next_expiry <= now) {
            return 0; // Immediate timeout
        }
        
        auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(next_expiry - now);
        return std::min(timeout_ms.count(), 1000L); // Max 1 second
    }
    
    void worker_thread() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(task_mutex_);
                task_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
                
                if (!running_ && task_queue_.empty()) {
                    break;
                }
                
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
            }
            
            if (task) {
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "Worker thread exception: " << e.what() << std::endl;
                }
            }
        }
    }
};

// =============================================================================
// EXAMPLE APPLICATIONS
// =============================================================================

class EchoServer {
private:
    EventLoop& event_loop_;
    int server_fd_;
    
public:
    EchoServer(EventLoop& loop, int port) : event_loop_(loop), server_fd_(-1) {
        // Mock server socket creation
        server_fd_ = 100; // Mock server fd
        std::cout << "Echo server created on port " << port << std::endl;
    }
    
    void start() {
        // Create server handler
        auto server_handler = std::make_shared<FdHandler>(server_fd_);
        
        server_handler->set_read_callback([this](const Event& event) {
            handle_new_connection(event);
        });
        
        server_handler->set_error_callback([](const Event& event) {
            std::cerr << "Server socket error on fd " << event.fd << std::endl;
        });
        
        event_loop_.add_fd(server_fd_, server_handler);
        std::cout << "Echo server started" << std::endl;
    }
    
private:
    void handle_new_connection(const Event& event) {
        // Mock accepting new connection
        int client_fd = 200 + rand() % 100; // Mock client fd
        std::cout << "New client connection: " << client_fd << std::endl;
        
        auto client_handler = std::make_shared<FdHandler>(client_fd);
        
        client_handler->set_read_callback([this](const Event& event) {
            handle_client_data(event);
        });
        
        client_handler->set_error_callback([this](const Event& event) {
            handle_client_disconnect(event);
        });
        
        event_loop_.add_fd(client_fd, client_handler);
    }
    
    void handle_client_data(const Event& event) {
        // Mock reading and echoing data
        std::cout << "Echoing data from client " << event.fd << std::endl;
        
        // In real implementation: read data, process it, and write back
        std::string mock_data = "Hello from client " + std::to_string(event.fd);
        std::cout << "Echo: " << mock_data << std::endl;
    }
    
    void handle_client_disconnect(const Event& event) {
        std::cout << "Client " << event.fd << " disconnected" << std::endl;
        event_loop_.remove_fd(event.fd);
    }
};

void demonstrate_timer_system(EventLoop& loop) {
    std::cout << "\n=== Timer System Demo ===" << std::endl;
    
    // One-shot timer
    loop.add_timer(std::chrono::milliseconds(1000), [] {
        std::cout << "One-shot timer fired!" << std::endl;
    });
    
    // Repeating timer
    loop.add_timer(std::chrono::milliseconds(2000), [] {
        std::cout << "Repeating timer tick..." << std::endl;
    }, true);
    
    // High-frequency timer
    int counter = 0;
    loop.add_timer(std::chrono::milliseconds(500), [&counter] {
        std::cout << "Fast timer: " << ++counter << std::endl;
    }, true);
}

void demonstrate_async_tasks(EventLoop& loop) {
    std::cout << "\n=== Async Task Demo ===" << std::endl;
    
    // Post async tasks
    for (int i = 0; i < 5; ++i) {
        loop.post_task([i] {
            std::cout << "Async task " << i << " executing in worker thread" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // CPU-intensive task
    loop.post_task([] {
        std::cout << "CPU-intensive task started" << std::endl;
        volatile long sum = 0;
        for (long i = 0; i < 1000000; ++i) {
            sum += i;
        }
        std::cout << "CPU-intensive task completed: " << sum << std::endl;
    });
}

int main() {
    std::cout << "EVENT LOOP WITH EPOLL\n";
    std::cout << "=====================\n";
    
    try {
        // Create event loop
        EventLoop event_loop(4);
        
        // Create echo server
        EchoServer echo_server(event_loop, 8080);
        echo_server.start();
        
        // Demonstrate timer system
        demonstrate_timer_system(event_loop);
        
        // Demonstrate async tasks
        demonstrate_async_tasks(event_loop);
        
        // Run event loop in separate thread for demo
        std::thread event_thread([&event_loop] {
            event_loop.run();
        });
        
        // Let it run for a while
        std::cout << "\nEvent loop running... (will stop in 10 seconds)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // Stop event loop
        event_loop.stop();
        event_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nKey Features:" << std::endl;
    std::cout << "• High-performance epoll-based I/O multiplexing" << std::endl;
    std::cout << "• Edge-triggered event handling" << std::endl;
    std::cout << "• Thread pool for callback execution" << std::endl;
    std::cout << "• Timer system with one-shot and repeating timers" << std::endl;
    std::cout << "• Async task posting and execution" << std::endl;
    std::cout << "• Scalable to thousands of concurrent connections" << std::endl;
    
    return 0;
}
