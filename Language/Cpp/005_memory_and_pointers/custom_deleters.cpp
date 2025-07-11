/*
 * MEMORY AND POINTERS - CUSTOM DELETERS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra custom_deleters.cpp -o custom_deleters
 */

#include <iostream>
#include <memory>
#include <functional>
#include <cstdio>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

class Resource {
    std::string name_;
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "Resource '" << name_ << "' created" << std::endl;
    }
    
    ~Resource() {
        std::cout << "Resource '" << name_ << "' destroyed" << std::endl;
    }
    
    const std::string& name() const { return name_; }
    void use() const { std::cout << "Using " << name_ << std::endl; }
};

// =============================================================================
// 1. FUNCTION POINTER DELETERS
// =============================================================================

void custom_delete_function(Resource* res) {
    std::cout << "Custom function deleting: " << res->name() << std::endl;
    delete res;
}

void demonstrate_function_pointer_deleters() {
    SECTION("Function Pointer Deleters");
    
    // unique_ptr with function pointer deleter
    std::unique_ptr<Resource, void(*)(Resource*)> ptr1(
        new Resource("FunctionDeleted"),
        custom_delete_function
    );
    
    ptr1->use();
    
    // Can also use function pointer directly
    std::unique_ptr<Resource, decltype(&custom_delete_function)> ptr2(
        new Resource("DirectFunction"),
        &custom_delete_function
    );
    
    ptr2->use();
    
    std::cout << "Objects will be deleted with custom function when going out of scope" << std::endl;
}

// =============================================================================
// 2. LAMBDA DELETERS
// =============================================================================

void demonstrate_lambda_deleters() {
    SECTION("Lambda Deleters");
    
    // Lambda deleter
    auto lambda_deleter = [](Resource* res) {
        std::cout << "Lambda deleting: " << res->name() << std::endl;
        delete res;
    };
    
    std::unique_ptr<Resource, decltype(lambda_deleter)> ptr1(
        new Resource("LambdaDeleted"),
        lambda_deleter
    );
    
    // Inline lambda
    auto ptr2 = std::unique_ptr<Resource, std::function<void(Resource*)>>(
        new Resource("InlineLambda"),
        [](Resource* res) {
            std::cout << "Inline lambda deleting: " << res->name() << std::endl;
            delete res;
        }
    );
    
    // Lambda with capture
    std::string prefix = "Captured";
    auto capturing_deleter = [prefix](Resource* res) {
        std::cout << prefix << " lambda deleting: " << res->name() << std::endl;
        delete res;
    };
    
    std::unique_ptr<Resource, decltype(capturing_deleter)> ptr3(
        new Resource("CapturedLambda"),
        capturing_deleter
    );
    
    ptr1->use();
    ptr2->use();
    ptr3->use();
}

// =============================================================================
// 3. FUNCTOR DELETERS
// =============================================================================

struct CustomDeleter {
    std::string message;
    
    CustomDeleter(const std::string& msg = "Default") : message(msg) {}
    
    void operator()(Resource* res) const {
        std::cout << message << " functor deleting: " << res->name() << std::endl;
        delete res;
    }
};

struct ArrayDeleter {
    void operator()(Resource* res) const {
        std::cout << "Array deleter called" << std::endl;
        delete[] res;
    }
};

void demonstrate_functor_deleters() {
    SECTION("Functor Deleters");
    
    // Custom functor deleter
    std::unique_ptr<Resource, CustomDeleter> ptr1(
        new Resource("FunctorDeleted"),
        CustomDeleter("Custom")
    );
    
    // Array deleter
    std::unique_ptr<Resource[], ArrayDeleter> ptr2(
        new Resource[3]{"Array1", "Array2", "Array3"},
        ArrayDeleter{}
    );
    
    ptr1->use();
    std::cout << "Array created with 3 elements" << std::endl;
}

// =============================================================================
// 4. SYSTEM RESOURCE DELETERS
// =============================================================================

// File handle deleter
struct FileDeleter {
    void operator()(FILE* file) const {
        if (file) {
            std::cout << "Closing file handle" << std::endl;
            fclose(file);
        }
    }
};

// Generic system resource deleter
template<typename T, void(*DeleteFunc)(T*)>
struct SystemDeleter {
    void operator()(T* resource) const {
        if (resource) {
            std::cout << "Cleaning up system resource" << std::endl;
            DeleteFunc(resource);
        }
    }
};

void cleanup_resource(void* ptr) {
    std::cout << "Generic cleanup function called" << std::endl;
    free(ptr);
}

void demonstrate_system_resource_deleters() {
    SECTION("System Resource Deleters");
    
    // File handle management
    std::unique_ptr<FILE, FileDeleter> file(
        fopen("test.txt", "w"),
        FileDeleter{}
    );
    
    if (file) {
        fprintf(file.get(), "Hello, custom deleters!\n");
        std::cout << "File written successfully" << std::endl;
    }
    
    // Generic system resource
    std::unique_ptr<void, SystemDeleter<void, cleanup_resource>> sys_res(
        malloc(1024),
        SystemDeleter<void, cleanup_resource>{}
    );
    
    if (sys_res) {
        std::cout << "System resource allocated" << std::endl;
    }
    
    // Lambda for system resources
    auto socket_deleter = [](int* socket_fd) {
        if (socket_fd && *socket_fd >= 0) {
            std::cout << "Closing socket: " << *socket_fd << std::endl;
            // close(*socket_fd);  // Would close actual socket
            delete socket_fd;
        }
    };
    
    std::unique_ptr<int, decltype(socket_deleter)> socket(
        new int(42),  // Simulate socket file descriptor
        socket_deleter
    );
    
    std::cout << "Socket handle: " << *socket << std::endl;
}

// =============================================================================
// 5. SHARED_PTR CUSTOM DELETERS
// =============================================================================

void demonstrate_shared_ptr_deleters() {
    SECTION("shared_ptr Custom Deleters");
    
    // shared_ptr with lambda deleter
    std::shared_ptr<Resource> ptr1(
        new Resource("SharedLambda"),
        [](Resource* res) {
            std::cout << "Shared lambda deleting: " << res->name() << std::endl;
            delete res;
        }
    );
    
    // Copy shared_ptr (deleter is shared)
    std::shared_ptr<Resource> ptr2 = ptr1;
    std::cout << "Reference count: " << ptr1.use_count() << std::endl;
    
    // shared_ptr with functor deleter
    std::shared_ptr<Resource> ptr3(
        new Resource("SharedFunctor"),
        CustomDeleter("Shared")
    );
    
    // shared_ptr with array deleter
    std::shared_ptr<Resource[]> ptr4(
        new Resource[2]{"Shared1", "Shared2"},
        [](Resource* res) {
            std::cout << "Shared array deleter" << std::endl;
            delete[] res;
        }
    );
    
    ptr1->use();
    ptr3->use();
    std::cout << "Shared array created" << std::endl;
}

// =============================================================================
// 6. CONDITIONAL DELETERS
// =============================================================================

enum class ResourceType { HEAP, STACK, EXTERNAL };

struct ConditionalDeleter {
    ResourceType type;
    
    ConditionalDeleter(ResourceType t) : type(t) {}
    
    void operator()(Resource* res) const {
        switch (type) {
            case ResourceType::HEAP:
                std::cout << "Deleting heap resource: " << res->name() << std::endl;
                delete res;
                break;
            case ResourceType::STACK:
                std::cout << "Stack resource, no deletion: " << res->name() << std::endl;
                break;
            case ResourceType::EXTERNAL:
                std::cout << "External resource, custom cleanup: " << res->name() << std::endl;
                // Custom cleanup logic
                break;
        }
    }
};

void demonstrate_conditional_deleters() {
    SECTION("Conditional Deleters");
    
    // Heap resource
    std::unique_ptr<Resource, ConditionalDeleter> heap_ptr(
        new Resource("HeapResource"),
        ConditionalDeleter(ResourceType::HEAP)
    );
    
    // Stack resource (no actual deletion)
    Resource stack_resource("StackResource");
    std::unique_ptr<Resource, ConditionalDeleter> stack_ptr(
        &stack_resource,
        ConditionalDeleter(ResourceType::STACK)
    );
    
    // External resource
    Resource external_resource("ExternalResource");
    std::unique_ptr<Resource, ConditionalDeleter> external_ptr(
        &external_resource,
        ConditionalDeleter(ResourceType::EXTERNAL)
    );
    
    heap_ptr->use();
    stack_ptr->use();
    external_ptr->use();
}

// =============================================================================
// 7. DELETER TYPE ERASURE
// =============================================================================

class TypeErasedDeleter {
private:
    std::function<void(void*)> deleter_;
    
public:
    template<typename T, typename Deleter>
    TypeErasedDeleter(Deleter&& del) {
        deleter_ = [del = std::forward<Deleter>(del)](void* ptr) {
            del(static_cast<T*>(ptr));
        };
    }
    
    void operator()(void* ptr) const {
        if (deleter_) {
            deleter_(ptr);
        }
    }
};

void demonstrate_type_erased_deleters() {
    SECTION("Type-Erased Deleters");
    
    // Different deleter types, same unique_ptr type
    using GenericPtr = std::unique_ptr<void, TypeErasedDeleter>;
    
    GenericPtr ptr1(
        new Resource("TypeErased1"),
        TypeErasedDeleter<Resource>([](Resource* res) {
            std::cout << "Type-erased lambda: " << res->name() << std::endl;
            delete res;
        })
    );
    
    GenericPtr ptr2(
        new Resource("TypeErased2"),
        TypeErasedDeleter<Resource>(CustomDeleter("TypeErased"))
    );
    
    std::cout << "Both pointers have same type despite different deleters" << std::endl;
}

// =============================================================================
// 8. BEST PRACTICES
// =============================================================================

void demonstrate_best_practices() {
    SECTION("Best Practices");
    
    std::cout << "Custom Deleter Best Practices:" << std::endl;
    std::cout << "1. Use lambdas for simple, one-off deleters" << std::endl;
    std::cout << "2. Use functors for reusable, configurable deleters" << std::endl;
    std::cout << "3. Use function pointers for C-style cleanup functions" << std::endl;
    std::cout << "4. Always check for null pointers in deleters" << std::endl;
    std::cout << "5. Consider exception safety in custom deleters" << std::endl;
    std::cout << "6. Use std::function for type erasure when needed" << std::endl;
    std::cout << "7. Prefer make_shared when possible, even with custom deleters" << std::endl;
    
    // Example of exception-safe deleter
    auto safe_deleter = [](Resource* res) noexcept {
        try {
            if (res) {
                std::cout << "Safe deleter: " << res->name() << std::endl;
                delete res;
            }
        } catch (...) {
            // Log error, but don't throw from destructor
            std::cout << "Error in deleter (handled safely)" << std::endl;
        }
    };
    
    std::unique_ptr<Resource, decltype(safe_deleter)> safe_ptr(
        new Resource("SafeDeleted"),
        safe_deleter
    );
    
    safe_ptr->use();
}

int main() {
    std::cout << "CUSTOM DELETERS TUTORIAL\n";
    std::cout << "========================\n";
    
    demonstrate_function_pointer_deleters();
    demonstrate_lambda_deleters();
    demonstrate_functor_deleters();
    demonstrate_system_resource_deleters();
    demonstrate_shared_ptr_deleters();
    demonstrate_conditional_deleters();
    demonstrate_type_erased_deleters();
    demonstrate_best_practices();
    
    return 0;
}

/*
KEY CONCEPTS:
- Custom deleters enable smart pointers to manage any resource
- Function pointers, lambdas, and functors can all be deleters
- shared_ptr stores deleter type-erased, unique_ptr in template parameter
- System resources (files, sockets, etc.) benefit from custom deleters
- Conditional deleters handle different resource types
- Exception safety is crucial in custom deleters
- Type erasure allows different deleter types in same container
- Always check for null and handle errors gracefully
*/
