/*
 * MEMORY AND POINTERS - MEMORY POOL ALLOCATOR
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra memory_pool_allocator.cpp -o memory_pool
 */

#include <iostream>
#include <vector>
#include <memory>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// Simple memory pool for fixed-size objects
template<typename T, size_t PoolSize>
class MemoryPool {
private:
    alignas(T) char pool_[sizeof(T) * PoolSize];
    bool used_[PoolSize];
    size_t next_free_;
    
public:
    MemoryPool() : next_free_(0) {
        std::fill(used_, used_ + PoolSize, false);
    }
    
    template<typename... Args>
    T* allocate(Args&&... args) {
        for (size_t i = 0; i < PoolSize; ++i) {
            size_t idx = (next_free_ + i) % PoolSize;
            if (!used_[idx]) {
                used_[idx] = true;
                next_free_ = (idx + 1) % PoolSize;
                void* ptr = pool_ + idx * sizeof(T);
                return new(ptr) T(std::forward<Args>(args)...);
            }
        }
        return nullptr;  // Pool exhausted
    }
    
    void deallocate(T* ptr) {
        if (!ptr) return;
        char* char_ptr = reinterpret_cast<char*>(ptr);
        if (char_ptr >= pool_ && char_ptr < pool_ + sizeof(pool_)) {
            size_t idx = (char_ptr - pool_) / sizeof(T);
            if (idx < PoolSize && used_[idx]) {
                ptr->~T();
                used_[idx] = false;
            }
        }
    }
    
    size_t available() const {
        return std::count(used_, used_ + PoolSize, false);
    }
};

class TestObject {
    int value_;
    static int count_;
public:
    TestObject(int val = 0) : value_(val) {
        ++count_;
        std::cout << "Object(" << value_ << ") created, total: " << count_ << std::endl;
    }
    
    ~TestObject() {
        --count_;
        std::cout << "Object(" << value_ << ") destroyed, remaining: " << count_ << std::endl;
    }
    
    int value() const { return value_; }
    static int count() { return count_; }
};

int TestObject::count_ = 0;

void demonstrate_memory_pool() {
    SECTION("Memory Pool Usage");
    
    MemoryPool<TestObject, 5> pool;
    std::vector<TestObject*> objects;
    
    // Allocate objects
    for (int i = 0; i < 7; ++i) {
        TestObject* obj = pool.allocate(i * 10);
        if (obj) {
            objects.push_back(obj);
        } else {
            std::cout << "Pool exhausted at allocation " << i << std::endl;
        }
    }
    
    std::cout << "Available slots: " << pool.available() << std::endl;
    
    // Deallocate some objects
    if (objects.size() > 2) {
        pool.deallocate(objects[1]);
        objects.erase(objects.begin() + 1);
    }
    
    std::cout << "After deallocation: " << pool.available() << " slots" << std::endl;
    
    // Allocate again
    TestObject* new_obj = pool.allocate(999);
    if (new_obj) {
        std::cout << "Reused slot for value: " << new_obj->value() << std::endl;
        objects.push_back(new_obj);
    }
    
    // Cleanup
    for (TestObject* obj : objects) {
        pool.deallocate(obj);
    }
}

// Stack allocator for temporary allocations
class StackAllocator {
    char* memory_;
    size_t size_;
    size_t offset_;
    
public:
    StackAllocator(size_t size) : size_(size), offset_(0) {
        memory_ = new char[size_];
    }
    
    ~StackAllocator() { delete[] memory_; }
    
    template<typename T, typename... Args>
    T* allocate(Args&&... args) {
        size_t space = size_ - offset_;
        void* ptr = memory_ + offset_;
        
        if (std::align(alignof(T), sizeof(T), ptr, space)) {
            offset_ = size_ - space + sizeof(T);
            return new(ptr) T(std::forward<Args>(args)...);
        }
        return nullptr;
    }
    
    void reset() { offset_ = 0; }
    size_t used() const { return offset_; }
};

void demonstrate_stack_allocator() {
    SECTION("Stack Allocator");
    
    StackAllocator stack(512);
    
    int* int_ptr = stack.allocate<int>(42);
    TestObject* obj_ptr = stack.allocate<TestObject>(100);
    double* double_ptr = stack.allocate<double>(3.14);
    
    if (int_ptr) std::cout << "Int: " << *int_ptr << std::endl;
    if (obj_ptr) std::cout << "Object: " << obj_ptr->value() << std::endl;
    if (double_ptr) std::cout << "Double: " << *double_ptr << std::endl;
    
    std::cout << "Stack used: " << stack.used() << " bytes" << std::endl;
    
    // Manual cleanup for demonstration
    if (obj_ptr) obj_ptr->~TestObject();
    
    stack.reset();
    std::cout << "Stack reset" << std::endl;
}

int main() {
    std::cout << "MEMORY POOL ALLOCATOR TUTORIAL\n";
    std::cout << "==============================\n";
    
    demonstrate_memory_pool();
    demonstrate_stack_allocator();
    
    std::cout << "\nFinal object count: " << TestObject::count() << std::endl;
    return 0;
}

/*
KEY CONCEPTS:
- Memory pools pre-allocate memory for fixed-size objects
- Reduces allocation overhead and fragmentation
- Stack allocators provide fast temporary allocation
- Placement new constructs objects in pre-allocated memory
- Manual lifetime management required
- Useful for performance-critical applications
- Trade memory for speed and predictability
*/
