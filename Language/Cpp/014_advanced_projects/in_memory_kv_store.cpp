/*
 * ADVANCED PROJECTS - IN-MEMORY KEY-VALUE STORE
 * 
 * A high-performance in-memory key-value store with threading, persistence,
 * expiration, and Redis-like commands.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread in_memory_kv_store.cpp -o kvstore
 */

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>
#include <sstream>
#include <queue>
#include <condition_variable>

// =============================================================================
// VALUE TYPES AND STORAGE
// =============================================================================

enum class ValueType {
    STRING, INTEGER, LIST, SET, HASH
};

class Value {
public:
    ValueType type;
    std::string string_val;
    int64_t int_val = 0;
    std::vector<std::string> list_val;
    std::unordered_set<std::string> set_val;
    std::unordered_map<std::string, std::string> hash_val;
    std::chrono::steady_clock::time_point expiry;
    bool has_expiry = false;
    
    Value(const std::string& s) : type(ValueType::STRING), string_val(s) {}
    Value(int64_t i) : type(ValueType::INTEGER), int_val(i) {}
    Value() : type(ValueType::STRING) {}
    
    bool is_expired() const {
        return has_expiry && std::chrono::steady_clock::now() > expiry;
    }
    
    void set_expiry(std::chrono::milliseconds ttl) {
        expiry = std::chrono::steady_clock::now() + ttl;
        has_expiry = true;
    }
    
    std::string to_string() const {
        switch (type) {
            case ValueType::STRING: return string_val;
            case ValueType::INTEGER: return std::to_string(int_val);
            case ValueType::LIST: return "[LIST:" + std::to_string(list_val.size()) + "]";
            case ValueType::SET: return "[SET:" + std::to_string(set_val.size()) + "]";
            case ValueType::HASH: return "[HASH:" + std::to_string(hash_val.size()) + "]";
        }
        return "";
    }
};

// =============================================================================
// THREAD-SAFE KEY-VALUE STORE
// =============================================================================

class InMemoryKVStore {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Value>> store_;
    
    // Statistics
    std::atomic<size_t> total_operations_;
    std::atomic<size_t> cache_hits_;
    std::atomic<size_t> cache_misses_;
    
    // Background cleanup
    std::thread cleanup_thread_;
    std::atomic<bool> running_;
    
    // Persistence
    std::string persist_file_;
    std::mutex persist_mutex_;
    
public:
    InMemoryKVStore(const std::string& persist_file = "kvstore.dat") 
        : total_operations_(0), cache_hits_(0), cache_misses_(0), 
          running_(true), persist_file_(persist_file) {
        
        std::cout << "Initializing KV Store with persistence: " << persist_file << std::endl;
        
        // Start background cleanup thread
        cleanup_thread_ = std::thread([this] { cleanup_expired_keys(); });
        
        // Load from persistence file
        load_from_disk();
    }
    
    ~InMemoryKVStore() {
        running_ = false;
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
        save_to_disk();
        std::cout << "KV Store destroyed" << std::endl;
    }
    
    // String operations
    bool set(const std::string& key, const std::string& value, 
             std::chrono::milliseconds ttl = std::chrono::milliseconds::zero()) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto val = std::make_shared<Value>(value);
        if (ttl.count() > 0) {
            val->set_expiry(ttl);
        }
        
        store_[key] = val;
        ++total_operations_;
        
        std::cout << "SET " << key << " = " << value;
        if (ttl.count() > 0) {
            std::cout << " (TTL: " << ttl.count() << "ms)";
        }
        std::cout << std::endl;
        
        return true;
    }
    
    std::string get(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired()) {
            ++cache_misses_;
            ++total_operations_;
            return "";
        }
        
        ++cache_hits_;
        ++total_operations_;
        
        std::cout << "GET " << key << " = " << it->second->to_string() << std::endl;
        return it->second->to_string();
    }
    
    bool del(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto erased = store_.erase(key);
        ++total_operations_;
        
        std::cout << "DEL " << key << " = " << (erased > 0 ? "OK" : "NOT_FOUND") << std::endl;
        return erased > 0;
    }
    
    bool exists(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        bool found = it != store_.end() && !it->second->is_expired();
        ++total_operations_;
        
        std::cout << "EXISTS " << key << " = " << (found ? "1" : "0") << std::endl;
        return found;
    }
    
    // Integer operations
    int64_t incr(const std::string& key, int64_t delta = 1) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired()) {
            auto val = std::make_shared<Value>(delta);
            store_[key] = val;
            ++total_operations_;
            
            std::cout << "INCR " << key << " = " << delta << " (new)" << std::endl;
            return delta;
        }
        
        if (it->second->type == ValueType::INTEGER) {
            it->second->int_val += delta;
            ++total_operations_;
            
            std::cout << "INCR " << key << " = " << it->second->int_val << std::endl;
            return it->second->int_val;
        }
        
        // Try to parse string as integer
        try {
            int64_t current = std::stoll(it->second->string_val);
            current += delta;
            it->second->type = ValueType::INTEGER;
            it->second->int_val = current;
            ++total_operations_;
            
            std::cout << "INCR " << key << " = " << current << std::endl;
            return current;
        } catch (...) {
            std::cout << "INCR " << key << " = ERROR (not a number)" << std::endl;
            return 0;
        }
    }
    
    // List operations
    size_t lpush(const std::string& key, const std::vector<std::string>& values) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired()) {
            auto val = std::make_shared<Value>();
            val->type = ValueType::LIST;
            store_[key] = val;
            it = store_.find(key);
        }
        
        if (it->second->type != ValueType::LIST) {
            std::cout << "LPUSH " << key << " = ERROR (not a list)" << std::endl;
            return 0;
        }
        
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit) {
            it->second->list_val.insert(it->second->list_val.begin(), *rit);
        }
        
        size_t new_size = it->second->list_val.size();
        ++total_operations_;
        
        std::cout << "LPUSH " << key << " = " << new_size << std::endl;
        return new_size;
    }
    
    std::string lpop(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired() || 
            it->second->type != ValueType::LIST || it->second->list_val.empty()) {
            ++total_operations_;
            std::cout << "LPOP " << key << " = (nil)" << std::endl;
            return "";
        }
        
        std::string result = it->second->list_val.front();
        it->second->list_val.erase(it->second->list_val.begin());
        ++total_operations_;
        
        std::cout << "LPOP " << key << " = " << result << std::endl;
        return result;
    }
    
    size_t llen(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired() || 
            it->second->type != ValueType::LIST) {
            ++total_operations_;
            std::cout << "LLEN " << key << " = 0" << std::endl;
            return 0;
        }
        
        size_t length = it->second->list_val.size();
        ++total_operations_;
        
        std::cout << "LLEN " << key << " = " << length << std::endl;
        return length;
    }
    
    // Set operations
    bool sadd(const std::string& key, const std::string& member) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired()) {
            auto val = std::make_shared<Value>();
            val->type = ValueType::SET;
            store_[key] = val;
            it = store_.find(key);
        }
        
        if (it->second->type != ValueType::SET) {
            std::cout << "SADD " << key << " = ERROR (not a set)" << std::endl;
            return false;
        }
        
        bool inserted = it->second->set_val.insert(member).second;
        ++total_operations_;
        
        std::cout << "SADD " << key << " " << member << " = " << (inserted ? "1" : "0") << std::endl;
        return inserted;
    }
    
    // Hash operations
    bool hset(const std::string& key, const std::string& field, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired()) {
            auto val = std::make_shared<Value>();
            val->type = ValueType::HASH;
            store_[key] = val;
            it = store_.find(key);
        }
        
        if (it->second->type != ValueType::HASH) {
            std::cout << "HSET " << key << " = ERROR (not a hash)" << std::endl;
            return false;
        }
        
        bool is_new = it->second->hash_val.find(field) == it->second->hash_val.end();
        it->second->hash_val[field] = value;
        ++total_operations_;
        
        std::cout << "HSET " << key << " " << field << " " << value 
                  << " = " << (is_new ? "1" : "0") << std::endl;
        return is_new;
    }
    
    std::string hget(const std::string& key, const std::string& field) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = store_.find(key);
        if (it == store_.end() || it->second->is_expired() || 
            it->second->type != ValueType::HASH) {
            ++total_operations_;
            std::cout << "HGET " << key << " " << field << " = (nil)" << std::endl;
            return "";
        }
        
        auto field_it = it->second->hash_val.find(field);
        if (field_it == it->second->hash_val.end()) {
            ++total_operations_;
            std::cout << "HGET " << key << " " << field << " = (nil)" << std::endl;
            return "";
        }
        
        ++total_operations_;
        std::cout << "HGET " << key << " " << field << " = " << field_it->second << std::endl;
        return field_it->second;
    }
    
    // Utility operations
    std::vector<std::string> keys(const std::string& pattern = "*") {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        std::vector<std::string> result;
        for (const auto& [key, value] : store_) {
            if (!value->is_expired() && (pattern == "*" || key.find(pattern) != std::string::npos)) {
                result.push_back(key);
            }
        }
        
        ++total_operations_;
        std::cout << "KEYS " << pattern << " = " << result.size() << " keys" << std::endl;
        return result;
    }
    
    void flushall() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        size_t count = store_.size();
        store_.clear();
        ++total_operations_;
        
        std::cout << "FLUSHALL = " << count << " keys removed" << std::endl;
    }
    
    // Statistics
    void info() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        size_t total_keys = 0;
        size_t expired_keys = 0;
        
        for (const auto& [key, value] : store_) {
            ++total_keys;
            if (value->is_expired()) {
                ++expired_keys;
            }
        }
        
        std::cout << "\n=== KV Store Info ===" << std::endl;
        std::cout << "Total keys: " << total_keys << std::endl;
        std::cout << "Expired keys: " << expired_keys << std::endl;
        std::cout << "Active keys: " << (total_keys - expired_keys) << std::endl;
        std::cout << "Total operations: " << total_operations_.load() << std::endl;
        std::cout << "Cache hits: " << cache_hits_.load() << std::endl;
        std::cout << "Cache misses: " << cache_misses_.load() << std::endl;
        
        if (cache_hits_.load() + cache_misses_.load() > 0) {
            double hit_rate = (double)cache_hits_.load() / (cache_hits_.load() + cache_misses_.load()) * 100.0;
            std::cout << "Hit rate: " << std::fixed << std::setprecision(2) << hit_rate << "%" << std::endl;
        }
        
        std::cout << "===================" << std::endl;
    }
    
private:
    void cleanup_expired_keys() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            if (!running_) break;
            
            std::unique_lock<std::shared_mutex> lock(mutex_);
            
            auto it = store_.begin();
            size_t removed = 0;
            
            while (it != store_.end()) {
                if (it->second->is_expired()) {
                    it = store_.erase(it);
                    ++removed;
                } else {
                    ++it;
                }
            }
            
            if (removed > 0) {
                std::cout << "Cleanup: removed " << removed << " expired keys" << std::endl;
            }
        }
    }
    
    void save_to_disk() {
        std::lock_guard<std::mutex> persist_lock(persist_mutex_);
        std::shared_lock<std::shared_mutex> store_lock(mutex_);
        
        std::ofstream file(persist_file_, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open persistence file for writing" << std::endl;
            return;
        }
        
        size_t count = 0;
        for (const auto& [key, value] : store_) {
            if (!value->is_expired()) {
                file << key << "\n";
                file << static_cast<int>(value->type) << "\n";
                file << value->to_string() << "\n";
                ++count;
            }
        }
        
        std::cout << "Saved " << count << " keys to disk" << std::endl;
    }
    
    void load_from_disk() {
        std::lock_guard<std::mutex> persist_lock(persist_mutex_);
        
        std::ifstream file(persist_file_);
        if (!file) {
            std::cout << "No persistence file found, starting fresh" << std::endl;
            return;
        }
        
        std::string key, type_str, value_str;
        size_t count = 0;
        
        while (std::getline(file, key) && std::getline(file, type_str) && std::getline(file, value_str)) {
            int type_int = std::stoi(type_str);
            auto value = std::make_shared<Value>(value_str);
            value->type = static_cast<ValueType>(type_int);
            
            store_[key] = value;
            ++count;
        }
        
        std::cout << "Loaded " << count << " keys from disk" << std::endl;
    }
};

// =============================================================================
// COMMAND INTERFACE
// =============================================================================

class KVStoreInterface {
private:
    InMemoryKVStore store_;
    
public:
    KVStoreInterface() = default;
    
    void run_interactive() {
        std::cout << "\n=== Interactive KV Store ===" << std::endl;
        std::cout << "Commands: SET, GET, DEL, EXISTS, INCR, LPUSH, LPOP, LLEN, SADD, HSET, HGET, KEYS, INFO, FLUSHALL, QUIT" << std::endl;
        std::cout << "Example: SET mykey myvalue" << std::endl;
        
        std::string line;
        while (true) {
            std::cout << "kvstore> ";
            if (!std::getline(std::cin, line)) break;
            
            if (line.empty()) continue;
            
            std::vector<std::string> tokens = split(line, ' ');
            if (tokens.empty()) continue;
            
            std::string command = to_upper(tokens[0]);
            
            if (command == "QUIT" || command == "EXIT") {
                break;
            } else if (command == "SET" && tokens.size() >= 3) {
                std::string key = tokens[1];
                std::string value = join(tokens, 2);
                store_.set(key, value);
            } else if (command == "GET" && tokens.size() >= 2) {
                std::string result = store_.get(tokens[1]);
                if (result.empty()) {
                    std::cout << "(nil)" << std::endl;
                }
            } else if (command == "DEL" && tokens.size() >= 2) {
                store_.del(tokens[1]);
            } else if (command == "EXISTS" && tokens.size() >= 2) {
                store_.exists(tokens[1]);
            } else if (command == "INCR" && tokens.size() >= 2) {
                store_.incr(tokens[1]);
            } else if (command == "LPUSH" && tokens.size() >= 3) {
                std::vector<std::string> values(tokens.begin() + 2, tokens.end());
                store_.lpush(tokens[1], values);
            } else if (command == "LPOP" && tokens.size() >= 2) {
                store_.lpop(tokens[1]);
            } else if (command == "LLEN" && tokens.size() >= 2) {
                store_.llen(tokens[1]);
            } else if (command == "SADD" && tokens.size() >= 3) {
                store_.sadd(tokens[1], tokens[2]);
            } else if (command == "HSET" && tokens.size() >= 4) {
                store_.hset(tokens[1], tokens[2], tokens[3]);
            } else if (command == "HGET" && tokens.size() >= 3) {
                store_.hget(tokens[1], tokens[2]);
            } else if (command == "KEYS") {
                std::string pattern = tokens.size() > 1 ? tokens[1] : "*";
                auto keys = store_.keys(pattern);
                for (size_t i = 0; i < keys.size(); ++i) {
                    std::cout << (i + 1) << ") " << keys[i] << std::endl;
                }
            } else if (command == "INFO") {
                store_.info();
            } else if (command == "FLUSHALL") {
                store_.flushall();
            } else {
                std::cout << "Unknown command or invalid syntax" << std::endl;
            }
        }
    }
    
    void run_demo() {
        std::cout << "\n=== KV Store Demo ===" << std::endl;
        
        // String operations
        store_.set("name", "Alice");
        store_.set("temp_key", "temporary", std::chrono::milliseconds(2000));
        store_.get("name");
        store_.exists("name");
        
        // Integer operations
        store_.incr("counter");
        store_.incr("counter", 5);
        store_.get("counter");
        
        // List operations
        store_.lpush("mylist", {"item1", "item2", "item3"});
        store_.llen("mylist");
        store_.lpop("mylist");
        store_.llen("mylist");
        
        // Set operations
        store_.sadd("myset", "member1");
        store_.sadd("myset", "member2");
        store_.sadd("myset", "member1"); // Duplicate
        
        // Hash operations
        store_.hset("user:1", "name", "Bob");
        store_.hset("user:1", "age", "30");
        store_.hget("user:1", "name");
        store_.hget("user:1", "age");
        
        // Utility operations
        store_.keys();
        store_.info();
        
        // Wait for expiration
        std::cout << "\nWaiting for key expiration..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        store_.get("temp_key"); // Should be expired
        
        store_.info();
    }
    
private:
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }
    
    std::string to_upper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    std::string join(const std::vector<std::string>& tokens, size_t start) {
        std::string result;
        for (size_t i = start; i < tokens.size(); ++i) {
            if (i > start) result += " ";
            result += tokens[i];
        }
        return result;
    }
};

int main() {
    std::cout << "IN-MEMORY KEY-VALUE STORE\n";
    std::cout << "=========================\n";
    
    try {
        KVStoreInterface interface;
        
        // Run demonstration
        interface.run_demo();
        
        // Interactive mode
        std::cout << "\nStarting interactive mode..." << std::endl;
        interface.run_interactive();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nKey Features:" << std::endl;
    std::cout << "• Thread-safe operations with shared_mutex" << std::endl;
    std::cout << "• Multiple data types (string, int, list, set, hash)" << std::endl;
    std::cout << "• TTL/expiration support with background cleanup" << std::endl;
    std::cout << "• Persistence to disk" << std::endl;
    std::cout << "• Redis-like command interface" << std::endl;
    std::cout << "• Performance statistics and monitoring" << std::endl;
    
    return 0;
}
