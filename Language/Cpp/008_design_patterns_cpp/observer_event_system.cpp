/*
 * DESIGN PATTERNS - OBSERVER PATTERN AND EVENT SYSTEM
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra observer_event_system.cpp -o observer
 */

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <string>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. CLASSIC OBSERVER PATTERN
// =============================================================================

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    
public:
    void attach(std::shared_ptr<Observer> observer) {
        observers_.push_back(observer);
    }
    
    void detach(std::shared_ptr<Observer> observer) {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&](const std::weak_ptr<Observer>& weak_obs) {
                    return weak_obs.expired() || weak_obs.lock() == observer;
                }), observers_.end());
    }
    
    void notify(const std::string& message) {
        // Clean up expired observers
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<Observer>& weak_obs) {
                    return weak_obs.expired();
                }), observers_.end());
        
        // Notify all active observers
        for (auto& weak_obs : observers_) {
            if (auto obs = weak_obs.lock()) {
                obs->update(message);
            }
        }
    }
};

class ConcreteObserver : public Observer {
private:
    std::string name_;
    
public:
    explicit ConcreteObserver(const std::string& name) : name_(name) {}
    
    void update(const std::string& message) override {
        std::cout << name_ << " received: " << message << std::endl;
    }
};

void demonstrate_classic_observer() {
    SECTION("Classic Observer Pattern");
    
    Subject subject;
    
    auto observer1 = std::make_shared<ConcreteObserver>("Observer1");
    auto observer2 = std::make_shared<ConcreteObserver>("Observer2");
    
    subject.attach(observer1);
    subject.attach(observer2);
    
    subject.notify("First message");
    
    subject.detach(observer1);
    subject.notify("Second message");
    
    // observer2 goes out of scope, should be cleaned up automatically
    observer2.reset();
    subject.notify("Third message");
}

// =============================================================================
// 2. MODERN EVENT SYSTEM WITH LAMBDAS
// =============================================================================

template<typename... Args>
class Event {
private:
    std::vector<std::function<void(Args...)>> handlers_;
    
public:
    void subscribe(std::function<void(Args...)> handler) {
        handlers_.push_back(handler);
    }
    
    void emit(Args... args) {
        for (auto& handler : handlers_) {
            handler(args...);
        }
    }
    
    void clear() {
        handlers_.clear();
    }
};

class EventPublisher {
public:
    Event<int> onValueChanged;
    Event<std::string> onMessageReceived;
    
private:
    int value_ = 0;
    
public:
    void setValue(int value) {
        value_ = value;
        onValueChanged.emit(value);
    }
    
    void sendMessage(const std::string& message) {
        onMessageReceived.emit(message);
    }
    
    int getValue() const { return value_; }
};

void demonstrate_modern_event_system() {
    SECTION("Modern Event System");
    
    EventPublisher publisher;
    
    // Subscribe with lambda
    publisher.onValueChanged.subscribe([](int value) {
        std::cout << "Value changed to: " << value << std::endl;
    });
    
    // Subscribe with function object
    publisher.onMessageReceived.subscribe([](const std::string& msg) {
        std::cout << "Message received: " << msg << std::endl;
    });
    
    // Multiple subscribers
    publisher.onValueChanged.subscribe([](int value) {
        std::cout << "Logger: Value is now " << value << std::endl;
    });
    
    publisher.setValue(42);
    publisher.sendMessage("Hello World");
    publisher.setValue(100);
}

// =============================================================================
// 3. TYPED EVENT SYSTEM
// =============================================================================

enum class EventType {
    USER_LOGIN,
    USER_LOGOUT,
    DATA_UPDATED,
    ERROR_OCCURRED
};

struct EventData {
    virtual ~EventData() = default;
};

struct LoginEvent : EventData {
    std::string username;
    explicit LoginEvent(const std::string& user) : username(user) {}
};

struct DataUpdateEvent : EventData {
    std::string table_name;
    int record_count;
    DataUpdateEvent(const std::string& table, int count) 
        : table_name(table), record_count(count) {}
};

class TypedEventSystem {
private:
    std::unordered_map<EventType, std::vector<std::function<void(const EventData&)>>> handlers_;
    
public:
    void subscribe(EventType type, std::function<void(const EventData&)> handler) {
        handlers_[type].push_back(handler);
    }
    
    void publish(EventType type, const EventData& data) {
        if (handlers_.find(type) != handlers_.end()) {
            for (auto& handler : handlers_[type]) {
                handler(data);
            }
        }
    }
};

void demonstrate_typed_event_system() {
    SECTION("Typed Event System");
    
    TypedEventSystem eventSystem;
    
    // Subscribe to login events
    eventSystem.subscribe(EventType::USER_LOGIN, [](const EventData& data) {
        const auto& loginEvent = static_cast<const LoginEvent&>(data);
        std::cout << "User logged in: " << loginEvent.username << std::endl;
    });
    
    // Subscribe to data update events
    eventSystem.subscribe(EventType::DATA_UPDATED, [](const EventData& data) {
        const auto& updateEvent = static_cast<const DataUpdateEvent&>(data);
        std::cout << "Data updated in " << updateEvent.table_name 
                  << ", " << updateEvent.record_count << " records" << std::endl;
    });
    
    // Publish events
    LoginEvent loginEvent("alice");
    eventSystem.publish(EventType::USER_LOGIN, loginEvent);
    
    DataUpdateEvent updateEvent("users", 150);
    eventSystem.publish(EventType::DATA_UPDATED, updateEvent);
}

// =============================================================================
// 4. SIGNAL-SLOT SYSTEM
// =============================================================================

template<typename... Args>
class Signal {
private:
    std::vector<std::function<void(Args...)>> slots_;
    
public:
    void connect(std::function<void(Args...)> slot) {
        slots_.push_back(slot);
    }
    
    void disconnect_all() {
        slots_.clear();
    }
    
    void emit(Args... args) {
        for (auto& slot : slots_) {
            slot(args...);
        }
    }
    
    size_t slot_count() const {
        return slots_.size();
    }
};

class Button {
public:
    Signal<> clicked;
    Signal<int, int> moved;
    
    void click() {
        std::cout << "Button clicked!" << std::endl;
        clicked.emit();
    }
    
    void move(int x, int y) {
        std::cout << "Button moved to (" << x << ", " << y << ")" << std::endl;
        moved.emit(x, y);
    }
};

void demonstrate_signal_slot() {
    SECTION("Signal-Slot System");
    
    Button button;
    
    // Connect slots
    button.clicked.connect([]() {
        std::cout << "Slot 1: Button was clicked!" << std::endl;
    });
    
    button.clicked.connect([]() {
        std::cout << "Slot 2: Handling click event" << std::endl;
    });
    
    button.moved.connect([](int x, int y) {
        std::cout << "Position tracker: Button at (" << x << ", " << y << ")" << std::endl;
    });
    
    // Emit signals
    button.click();
    button.move(10, 20);
    
    std::cout << "Connected slots: " << button.clicked.slot_count() << std::endl;
}

// =============================================================================
// 5. ASYNCHRONOUS EVENT SYSTEM
// =============================================================================

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class AsyncEventSystem {
private:
    struct Event {
        std::function<void()> handler;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    std::queue<Event> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};
    std::thread worker_thread_;
    
    void worker() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !event_queue_.empty() || !running_; });
            
            if (!running_) break;
            
            Event event = event_queue_.front();
            event_queue_.pop();
            lock.unlock();
            
            event.handler();
        }
    }
    
public:
    AsyncEventSystem() : worker_thread_(&AsyncEventSystem::worker, this) {}
    
    ~AsyncEventSystem() {
        running_ = false;
        cv_.notify_all();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
    
    void post_event(std::function<void()> handler) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            event_queue_.push({handler, std::chrono::steady_clock::now()});
        }
        cv_.notify_one();
    }
    
    size_t pending_events() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return event_queue_.size();
    }
};

void demonstrate_async_event_system() {
    SECTION("Asynchronous Event System");
    
    AsyncEventSystem asyncSystem;
    
    // Post some events
    for (int i = 1; i <= 5; ++i) {
        asyncSystem.post_event([i]() {
            std::cout << "Processing async event " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Wait a bit for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    std::cout << "Pending events: " << asyncSystem.pending_events() << std::endl;
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison");
    
    const int iterations = 1000000;
    
    // Direct function call
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile int x = i * 2;  // Prevent optimization
        (void)x;
    }
    auto direct_time = std::chrono::high_resolution_clock::now() - start;
    
    // Event system overhead
    Event<int> event;
    event.subscribe([](int x) { volatile int y = x * 2; (void)y; });
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        event.emit(i);
    }
    auto event_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Direct calls: " << std::chrono::duration_cast<std::chrono::microseconds>(direct_time).count() << " μs" << std::endl;
    std::cout << "Event system: " << std::chrono::duration_cast<std::chrono::microseconds>(event_time).count() << " μs" << std::endl;
    
    double overhead = static_cast<double>(event_time.count()) / direct_time.count();
    std::cout << "Overhead factor: " << overhead << "x" << std::endl;
}

int main() {
    std::cout << "OBSERVER PATTERN AND EVENT SYSTEMS\n";
    std::cout << "==================================\n";
    
    demonstrate_classic_observer();
    demonstrate_modern_event_system();
    demonstrate_typed_event_system();
    demonstrate_signal_slot();
    demonstrate_async_event_system();
    performance_comparison();
    
    std::cout << "\nPattern Comparison:" << std::endl;
    std::cout << "Classic Observer: OOP-based, virtual functions" << std::endl;
    std::cout << "Modern Events: Function-based, type-safe" << std::endl;
    std::cout << "Signal-Slot: Qt-style, easy to use" << std::endl;
    std::cout << "Async Events: Non-blocking, queued processing" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Observer pattern decouples subjects from observers
- Modern event systems use std::function for flexibility
- Weak pointers prevent memory leaks in observer relationships
- Signal-slot systems provide intuitive connect/emit interface
- Async event systems enable non-blocking communication
- Type safety prevents runtime errors in event handling
- Performance overhead varies with implementation complexity
- Choose pattern based on requirements: sync/async, type safety, performance
*/
