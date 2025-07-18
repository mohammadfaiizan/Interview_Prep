/*
 * DESIGN PATTERNS - THREAD-SAFE SINGLETON
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread singleton_thread_safe.cpp -o singleton
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. NAIVE SINGLETON (NOT THREAD-SAFE)
// =============================================================================

class NaiveSingleton {
private:
    static NaiveSingleton* instance_;
    
    NaiveSingleton() { std::cout << "NaiveSingleton created\n"; }
    
public:
    static NaiveSingleton* getInstance() {
        if (instance_ == nullptr) {
            instance_ = new NaiveSingleton();
        }
        return instance_;
    }
    
    void doSomething() { std::cout << "Doing something...\n"; }
    
    // Prevent copying
    NaiveSingleton(const NaiveSingleton&) = delete;
    NaiveSingleton& operator=(const NaiveSingleton&) = delete;
};

NaiveSingleton* NaiveSingleton::instance_ = nullptr;

// =============================================================================
// 2. MUTEX-BASED THREAD-SAFE SINGLETON
// =============================================================================

class MutexSingleton {
private:
    static MutexSingleton* instance_;
    static std::mutex mutex_;
    
    MutexSingleton() { std::cout << "MutexSingleton created\n"; }
    
public:
    static MutexSingleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new MutexSingleton();
        }
        return instance_;
    }
    
    void doSomething() { std::cout << "Mutex singleton working...\n"; }
    
    MutexSingleton(const MutexSingleton&) = delete;
    MutexSingleton& operator=(const MutexSingleton&) = delete;
};

MutexSingleton* MutexSingleton::instance_ = nullptr;
std::mutex MutexSingleton::mutex_;

// =============================================================================
// 3. DOUBLE-CHECKED LOCKING SINGLETON
// =============================================================================

class DoubleCheckedSingleton {
private:
    static std::atomic<DoubleCheckedSingleton*> instance_;
    static std::mutex mutex_;
    
    DoubleCheckedSingleton() { std::cout << "DoubleCheckedSingleton created\n"; }
    
public:
    static DoubleCheckedSingleton* getInstance() {
        DoubleCheckedSingleton* tmp = instance_.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            tmp = instance_.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new DoubleCheckedSingleton();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }
    
    void doSomething() { std::cout << "Double-checked singleton working...\n"; }
    
    DoubleCheckedSingleton(const DoubleCheckedSingleton&) = delete;
    DoubleCheckedSingleton& operator=(const DoubleCheckedSingleton&) = delete;
};

std::atomic<DoubleCheckedSingleton*> DoubleCheckedSingleton::instance_{nullptr};
std::mutex DoubleCheckedSingleton::mutex_;

// =============================================================================
// 4. MEYER'S SINGLETON (C++11 THREAD-SAFE)
// =============================================================================

class MeyersSingleton {
private:
    MeyersSingleton() { std::cout << "MeyersSingleton created\n"; }
    
public:
    static MeyersSingleton& getInstance() {
        static MeyersSingleton instance;  // Thread-safe in C++11+
        return instance;
    }
    
    void doSomething() { std::cout << "Meyer's singleton working...\n"; }
    
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
};

// =============================================================================
// 5. CALL_ONCE SINGLETON
// =============================================================================

class CallOnceSingleton {
private:
    static CallOnceSingleton* instance_;
    static std::once_flag once_flag_;
    
    CallOnceSingleton() { std::cout << "CallOnceSingleton created\n"; }
    
    static void createInstance() {
        instance_ = new CallOnceSingleton();
    }
    
public:
    static CallOnceSingleton* getInstance() {
        std::call_once(once_flag_, createInstance);
        return instance_;
    }
    
    void doSomething() { std::cout << "Call-once singleton working...\n"; }
    
    CallOnceSingleton(const CallOnceSingleton&) = delete;
    CallOnceSingleton& operator=(const CallOnceSingleton&) = delete;
};

CallOnceSingleton* CallOnceSingleton::instance_ = nullptr;
std::once_flag CallOnceSingleton::once_flag_;

// =============================================================================
// 6. TESTING THREAD SAFETY
// =============================================================================

template<typename SingletonType>
void test_thread_safety(const std::string& name, int num_threads = 10) {
    SECTION("Testing " + name);
    
    std::vector<SingletonType*> instances(num_threads);
    std::vector<std::thread> threads;
    
    // Create threads that get singleton instance
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&instances, i]() {
            if constexpr (std::is_same_v<SingletonType, MeyersSingleton>) {
                instances[i] = &SingletonType::getInstance();
            } else {
                instances[i] = SingletonType::getInstance();
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Check if all instances are the same
    bool all_same = true;
    for (int i = 1; i < num_threads; ++i) {
        if (instances[i] != instances[0]) {
            all_same = false;
            break;
        }
    }
    
    std::cout << name << " thread safety: " << (all_same ? "PASS" : "FAIL") << std::endl;
    std::cout << "All instances point to same object: " << all_same << std::endl;
}

// =============================================================================
// 7. PERFORMANCE COMPARISON
// =============================================================================

template<typename SingletonType>
void benchmark_singleton(const std::string& name, int iterations = 1000000) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        if constexpr (std::is_same_v<SingletonType, MeyersSingleton>) {
            volatile auto* instance = &SingletonType::getInstance();
            (void)instance;  // Prevent optimization
        } else {
            volatile auto* instance = SingletonType::getInstance();
            (void)instance;  // Prevent optimization
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << name << " (" << iterations << " calls): " 
              << duration.count() << " μs" << std::endl;
}

void performance_comparison() {
    SECTION("Performance Comparison");
    
    // Initialize singletons first
    MutexSingleton::getInstance();
    DoubleCheckedSingleton::getInstance();
    MeyersSingleton::getInstance();
    CallOnceSingleton::getInstance();
    
    benchmark_singleton<MutexSingleton>("Mutex Singleton");
    benchmark_singleton<DoubleCheckedSingleton>("Double-Checked Singleton");
    benchmark_singleton<MeyersSingleton>("Meyer's Singleton");
    benchmark_singleton<CallOnceSingleton>("Call-Once Singleton");
}

// =============================================================================
// 8. RAII SINGLETON WITH SMART POINTERS
// =============================================================================

class SmartSingleton {
private:
    static std::unique_ptr<SmartSingleton> instance_;
    static std::once_flag once_flag_;
    
    SmartSingleton() { std::cout << "SmartSingleton created\n"; }
    
    static void createInstance() {
        instance_ = std::make_unique<SmartSingleton>();
    }
    
public:
    static SmartSingleton& getInstance() {
        std::call_once(once_flag_, createInstance);
        return *instance_;
    }
    
    void doSomething() { std::cout << "Smart singleton working...\n"; }
    
    SmartSingleton(const SmartSingleton&) = delete;
    SmartSingleton& operator=(const SmartSingleton&) = delete;
};

std::unique_ptr<SmartSingleton> SmartSingleton::instance_;
std::once_flag SmartSingleton::once_flag_;

// =============================================================================
// 9. SINGLETON REGISTRY PATTERN
// =============================================================================

template<typename T>
class SingletonRegistry {
private:
    static std::unordered_map<std::string, std::unique_ptr<T>> instances_;
    static std::mutex mutex_;
    
public:
    template<typename... Args>
    static T& getInstance(const std::string& key, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = instances_.find(key);
        if (it == instances_.end()) {
            instances_[key] = std::make_unique<T>(std::forward<Args>(args)...);
            return *instances_[key];
        }
        return *it->second;
    }
    
    static void removeInstance(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        instances_.erase(key);
    }
    
    static size_t getInstanceCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return instances_.size();
    }
};

template<typename T>
std::unordered_map<std::string, std::unique_ptr<T>> SingletonRegistry<T>::instances_;

template<typename T>
std::mutex SingletonRegistry<T>::mutex_;

class ConfigManager {
private:
    std::string config_name_;
    
public:
    explicit ConfigManager(const std::string& name) : config_name_(name) {
        std::cout << "ConfigManager '" << name << "' created\n";
    }
    
    const std::string& getName() const { return config_name_; }
};

void demonstrate_singleton_registry() {
    SECTION("Singleton Registry Pattern");
    
    auto& config1 = SingletonRegistry<ConfigManager>::getInstance("database", "db_config");
    auto& config2 = SingletonRegistry<ConfigManager>::getInstance("cache", "cache_config");
    auto& config3 = SingletonRegistry<ConfigManager>::getInstance("database", "should_not_create");
    
    std::cout << "Config1 name: " << config1.getName() << std::endl;
    std::cout << "Config2 name: " << config2.getName() << std::endl;
    std::cout << "Config3 name: " << config3.getName() << std::endl;
    std::cout << "Config1 and Config3 are same: " << (&config1 == &config3) << std::endl;
    std::cout << "Total instances: " << SingletonRegistry<ConfigManager>::getInstanceCount() << std::endl;
}

int main() {
    std::cout << "THREAD-SAFE SINGLETON PATTERNS\n";
    std::cout << "==============================\n";
    
    // Test thread safety
    test_thread_safety<MutexSingleton>("Mutex Singleton");
    test_thread_safety<DoubleCheckedSingleton>("Double-Checked Singleton");
    test_thread_safety<MeyersSingleton>("Meyer's Singleton");
    test_thread_safety<CallOnceSingleton>("Call-Once Singleton");
    
    // Performance comparison
    performance_comparison();
    
    // Smart pointer singleton
    SECTION("Smart Pointer Singleton");
    SmartSingleton::getInstance().doSomething();
    
    // Registry pattern
    demonstrate_singleton_registry();
    
    std::cout << "\nRecommendations:" << std::endl;
    std::cout << "1. Use Meyer's Singleton for simplicity (C++11+)" << std::endl;
    std::cout << "2. Use std::call_once for explicit control" << std::endl;
    std::cout << "3. Avoid double-checked locking unless necessary" << std::endl;
    std::cout << "4. Consider dependency injection instead of singletons" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Meyer's Singleton is simplest and thread-safe in C++11+
- Double-checked locking requires careful memory ordering
- std::call_once provides explicit initialization control
- Mutex-based approach is simple but slower
- Smart pointers enable RAII and automatic cleanup
- Registry pattern allows multiple named instances
- Performance varies: Meyer's ≈ Double-checked > Call-once > Mutex
- Consider alternatives like dependency injection for testability
*/
