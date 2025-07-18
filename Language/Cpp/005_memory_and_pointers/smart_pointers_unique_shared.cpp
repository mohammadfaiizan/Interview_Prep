/*
 * MEMORY AND POINTERS - SMART POINTERS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra smart_pointers_unique_shared.cpp -o smart_pointers
 */

#include <iostream>
#include <memory>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

class Resource {
    std::string name_;
    static int count_;
public:
    Resource(const std::string& name) : name_(name) {
        ++count_;
        std::cout << "Resource '" << name_ << "' created (total: " << count_ << ")" << std::endl;
    }
    
    ~Resource() {
        --count_;
        std::cout << "Resource '" << name_ << "' destroyed (remaining: " << count_ << ")" << std::endl;
    }
    
    void use() const { std::cout << "Using " << name_ << std::endl; }
    const std::string& name() const { return name_; }
    static int count() { return count_; }
};

int Resource::count_ = 0;

// =============================================================================
// 1. UNIQUE_PTR - EXCLUSIVE OWNERSHIP
// =============================================================================

void demonstrate_unique_ptr() {
    SECTION("std::unique_ptr - Exclusive Ownership");
    
    // Creation
    auto ptr1 = std::make_unique<Resource>("Resource1");
    std::unique_ptr<Resource> ptr2(new Resource("Resource2"));
    
    // Usage
    ptr1->use();
    (*ptr1).use();
    
    // Move semantics (unique_ptr is move-only)
    auto ptr3 = std::move(ptr1);
    if (!ptr1) std::cout << "ptr1 is empty after move" << std::endl;
    if (ptr3) std::cout << "ptr3 owns the resource" << std::endl;
    
    // Reset and release
    ptr2.reset();  // Destroys resource
    Resource* raw = ptr3.release();  // Releases ownership
    delete raw;  // Must manually delete
    
    // Array version
    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) arr[i] = i;
}

// =============================================================================
// 2. SHARED_PTR - SHARED OWNERSHIP
// =============================================================================

void demonstrate_shared_ptr() {
    SECTION("std::shared_ptr - Shared Ownership");
    
    // Creation
    auto ptr1 = std::make_shared<Resource>("SharedResource");
    std::cout << "Reference count: " << ptr1.use_count() << std::endl;
    
    // Copy (increases reference count)
    auto ptr2 = ptr1;
    auto ptr3 = ptr1;
    std::cout << "Reference count with 3 copies: " << ptr1.use_count() << std::endl;
    
    // All point to same object
    ptr1->use();
    ptr2->use();
    ptr3->use();
    
    // Reset decreases count
    ptr2.reset();
    std::cout << "Count after reset: " << ptr1.use_count() << std::endl;
    
    // From unique_ptr
    auto unique_res = std::make_unique<Resource>("FromUnique");
    auto shared_res = std::shared_ptr<Resource>(std::move(unique_res));
}

// =============================================================================
// 3. WEAK_PTR - NON-OWNING REFERENCE
// =============================================================================

void demonstrate_weak_ptr() {
    SECTION("std::weak_ptr - Non-owning Reference");
    
    std::weak_ptr<Resource> weak1;
    
    {
        auto shared1 = std::make_shared<Resource>("WeakResource");
        weak1 = shared1;
        
        std::cout << "Shared count: " << shared1.use_count() << std::endl;
        std::cout << "Weak expired: " << weak1.expired() << std::endl;
        
        // Lock to get shared_ptr
        if (auto locked = weak1.lock()) {
            locked->use();
        }
    }  // shared1 destroyed here
    
    std::cout << "Weak expired after destruction: " << weak1.expired() << std::endl;
    
    if (auto locked = weak1.lock()) {
        std::cout << "Locked successfully" << std::endl;
    } else {
        std::cout << "Failed to lock expired weak_ptr" << std::endl;
    }
}

// =============================================================================
// 4. CIRCULAR REFERENCES
// =============================================================================

struct Node : public std::enable_shared_from_this<Node> {
    std::string name;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> parent;  // Break cycle with weak_ptr
    
    Node(const std::string& n) : name(n) {}
    ~Node() { std::cout << "Node " << name << " destroyed" << std::endl; }
    
    void set_next(std::shared_ptr<Node> n) {
        next = n;
        if (n) n->parent = shared_from_this();
    }
};

void demonstrate_circular_references() {
    SECTION("Circular References");
    
    {
        auto node1 = std::make_shared<Node>("A");
        auto node2 = std::make_shared<Node>("B");
        
        node1->set_next(node2);
        // node2->next = node1;  // Would create cycle without weak_ptr
        
        std::cout << "Node1 count: " << node1.use_count() << std::endl;
        std::cout << "Node2 count: " << node2.use_count() << std::endl;
    }  // Nodes destroyed properly due to weak_ptr
}

// =============================================================================
// 5. CUSTOM DELETERS
// =============================================================================

void demonstrate_custom_deleters() {
    SECTION("Custom Deleters");
    
    // unique_ptr with lambda deleter
    auto ptr1 = std::unique_ptr<Resource, void(*)(Resource*)>(
        new Resource("CustomDeleted"),
        [](Resource* r) { std::cout << "Custom delete\n"; delete r; }
    );
    
    // shared_ptr with custom deleter
    auto ptr2 = std::shared_ptr<Resource>(
        new Resource("SharedCustom"),
        [](Resource* r) { std::cout << "Shared custom delete\n"; delete r; }
    );
    
    // File handle example
    auto file = std::unique_ptr<FILE, int(*)(FILE*)>(
        fopen("test.txt", "w"),
        [](FILE* f) { return f ? fclose(f) : 0; }
    );
    
    if (file) {
        fprintf(file.get(), "Hello!\n");
    }
}

// =============================================================================
// 6. BEST PRACTICES
// =============================================================================

std::unique_ptr<Resource> create_resource(const std::string& name) {
    return std::make_unique<Resource>(name);
}

void use_resource(const std::unique_ptr<Resource>& res) {
    if (res) res->use();
}

void take_ownership(std::unique_ptr<Resource> res) {
    if (res) res->use();
}

void demonstrate_best_practices() {
    SECTION("Best Practices");
    
    std::cout << "1. Prefer make_unique/make_shared" << std::endl;
    auto res1 = std::make_unique<Resource>("Best1");
    auto res2 = std::make_shared<Resource>("Best2");
    
    std::cout << "2. Factory functions return smart pointers" << std::endl;
    auto factory_res = create_resource("Factory");
    
    std::cout << "3. Function parameters:" << std::endl;
    use_resource(factory_res);  // By reference for use
    // take_ownership(std::move(factory_res));  // By value for ownership
    
    std::cout << "4. Use unique_ptr by default, shared_ptr when sharing needed" << std::endl;
    std::cout << "5. Use weak_ptr to break cycles" << std::endl;
    std::cout << "6. Don't mix smart and raw pointer ownership" << std::endl;
}

// =============================================================================
// 7. PERFORMANCE
// =============================================================================

void demonstrate_performance() {
    SECTION("Performance");
    
    std::cout << "Memory overhead:" << std::endl;
    std::cout << "Raw pointer: " << sizeof(Resource*) << " bytes" << std::endl;
    std::cout << "unique_ptr: " << sizeof(std::unique_ptr<Resource>) << " bytes" << std::endl;
    std::cout << "shared_ptr: " << sizeof(std::shared_ptr<Resource>) << " bytes" << std::endl;
    std::cout << "weak_ptr: " << sizeof(std::weak_ptr<Resource>) << " bytes" << std::endl;
    
    std::cout << "\nPerformance:" << std::endl;
    std::cout << "unique_ptr: minimal overhead, similar to raw pointer" << std::endl;
    std::cout << "shared_ptr: reference counting overhead" << std::endl;
    std::cout << "make_shared: more efficient than shared_ptr(new T)" << std::endl;
}

int main() {
    std::cout << "SMART POINTERS TUTORIAL\n";
    std::cout << "=======================\n";
    
    demonstrate_unique_ptr();
    demonstrate_shared_ptr();
    demonstrate_weak_ptr();
    demonstrate_circular_references();
    demonstrate_custom_deleters();
    demonstrate_best_practices();
    demonstrate_performance();
    
    std::cout << "\nFinal resource count: " << Resource::count() << std::endl;
    return 0;
}

/*
KEY CONCEPTS:
- unique_ptr: exclusive ownership, move-only, minimal overhead
- shared_ptr: shared ownership, reference counting
- weak_ptr: non-owning reference, breaks cycles
- make_unique/make_shared: preferred creation
- Custom deleters for special cleanup
- RAII: automatic resource management
- Performance: unique_ptr ≈ raw, shared_ptr has overhead
- Best practice: unique_ptr by default, shared_ptr when sharing
*/
