/*
 * MEMORY AND POINTERS - MEMORY ALIGNMENT AND PADDING
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra memory_alignment_padding.cpp -o memory_alignment
 */

#include <iostream>
#include <cstddef>
#include <memory>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC ALIGNMENT CONCEPTS
// =============================================================================

void demonstrate_basic_alignment() {
    SECTION("Basic Alignment Concepts");
    
    std::cout << "Fundamental type alignments:" << std::endl;
    std::cout << "char: " << alignof(char) << " bytes" << std::endl;
    std::cout << "short: " << alignof(short) << " bytes" << std::endl;
    std::cout << "int: " << alignof(int) << " bytes" << std::endl;
    std::cout << "long: " << alignof(long) << " bytes" << std::endl;
    std::cout << "long long: " << alignof(long long) << " bytes" << std::endl;
    std::cout << "float: " << alignof(float) << " bytes" << std::endl;
    std::cout << "double: " << alignof(double) << " bytes" << std::endl;
    std::cout << "void*: " << alignof(void*) << " bytes" << std::endl;
    
    // Check if addresses are aligned
    int x = 42;
    std::cout << "\nAddress of int x: " << &x << std::endl;
    std::cout << "Is aligned to 4 bytes: " << (reinterpret_cast<uintptr_t>(&x) % 4 == 0) << std::endl;
}

// =============================================================================
// 2. STRUCT PADDING
// =============================================================================

struct UnalignedStruct {
    char a;     // 1 byte
    int b;      // 4 bytes
    char c;     // 1 byte
    double d;   // 8 bytes
};

struct AlignedStruct {
    double d;   // 8 bytes
    int b;      // 4 bytes
    char a;     // 1 byte
    char c;     // 1 byte
};

#pragma pack(push, 1)
struct PackedStruct {
    char a;     // 1 byte
    int b;      // 4 bytes
    char c;     // 1 byte
    double d;   // 8 bytes
};
#pragma pack(pop)

void demonstrate_struct_padding() {
    SECTION("Struct Padding");
    
    std::cout << "Struct sizes and member offsets:" << std::endl;
    
    std::cout << "\nUnalignedStruct (char, int, char, double):" << std::endl;
    std::cout << "Size: " << sizeof(UnalignedStruct) << " bytes" << std::endl;
    std::cout << "Alignment: " << alignof(UnalignedStruct) << " bytes" << std::endl;
    std::cout << "Offset of a: " << offsetof(UnalignedStruct, a) << std::endl;
    std::cout << "Offset of b: " << offsetof(UnalignedStruct, b) << std::endl;
    std::cout << "Offset of c: " << offsetof(UnalignedStruct, c) << std::endl;
    std::cout << "Offset of d: " << offsetof(UnalignedStruct, d) << std::endl;
    
    std::cout << "\nAlignedStruct (double, int, char, char):" << std::endl;
    std::cout << "Size: " << sizeof(AlignedStruct) << " bytes" << std::endl;
    std::cout << "Alignment: " << alignof(AlignedStruct) << " bytes" << std::endl;
    
    std::cout << "\nPackedStruct (packed to 1 byte):" << std::endl;
    std::cout << "Size: " << sizeof(PackedStruct) << " bytes" << std::endl;
    std::cout << "Alignment: " << alignof(PackedStruct) << " bytes" << std::endl;
    
    // Memory layout visualization
    UnalignedStruct us;
    std::cout << "\nMemory addresses in UnalignedStruct:" << std::endl;
    std::cout << "Base: " << &us << std::endl;
    std::cout << "a: " << (void*)&us.a << " (+" << ((char*)&us.a - (char*)&us) << ")" << std::endl;
    std::cout << "b: " << (void*)&us.b << " (+" << ((char*)&us.b - (char*)&us) << ")" << std::endl;
    std::cout << "c: " << (void*)&us.c << " (+" << ((char*)&us.c - (char*)&us) << ")" << std::endl;
    std::cout << "d: " << (void*)&us.d << " (+" << ((char*)&us.d - (char*)&us) << ")" << std::endl;
}

// =============================================================================
// 3. ALIGNAS AND ALIGNOF
// =============================================================================

struct alignas(16) AlignedTo16 {
    int a;
    int b;
};

struct alignas(64) CacheLineAligned {
    int data[16];  // 64 bytes total
};

void demonstrate_alignas_alignof() {
    SECTION("alignas and alignof");
    
    std::cout << "Custom alignment with alignas:" << std::endl;
    std::cout << "AlignedTo16 size: " << sizeof(AlignedTo16) << std::endl;
    std::cout << "AlignedTo16 alignment: " << alignof(AlignedTo16) << std::endl;
    
    std::cout << "CacheLineAligned size: " << sizeof(CacheLineAligned) << std::endl;
    std::cout << "CacheLineAligned alignment: " << alignof(CacheLineAligned) << std::endl;
    
    // Check actual alignment
    AlignedTo16 obj16;
    CacheLineAligned objCache;
    
    std::cout << "\nActual addresses:" << std::endl;
    std::cout << "AlignedTo16: " << &obj16 << std::endl;
    std::cout << "Is 16-byte aligned: " << (reinterpret_cast<uintptr_t>(&obj16) % 16 == 0) << std::endl;
    
    std::cout << "CacheLineAligned: " << &objCache << std::endl;
    std::cout << "Is 64-byte aligned: " << (reinterpret_cast<uintptr_t>(&objCache) % 64 == 0) << std::endl;
}

// =============================================================================
// 4. DYNAMIC ALLOCATION AND ALIGNMENT
// =============================================================================

void demonstrate_dynamic_alignment() {
    SECTION("Dynamic Allocation and Alignment");
    
    // Regular new
    int* regular_ptr = new int(42);
    std::cout << "Regular new alignment: " << (reinterpret_cast<uintptr_t>(regular_ptr) % alignof(int)) << std::endl;
    
    // Aligned allocation (C++17)
    void* aligned_ptr = std::aligned_alloc(64, 128);  // 128 bytes aligned to 64
    if (aligned_ptr) {
        std::cout << "Aligned alloc address: " << aligned_ptr << std::endl;
        std::cout << "Is 64-byte aligned: " << (reinterpret_cast<uintptr_t>(aligned_ptr) % 64 == 0) << std::endl;
        std::free(aligned_ptr);
    }
    
    // Placement new with alignment
    alignas(32) char buffer[128];
    int* placed_ptr = new(buffer) int(42);
    std::cout << "Placement new address: " << placed_ptr << std::endl;
    std::cout << "Is 32-byte aligned: " << (reinterpret_cast<uintptr_t>(placed_ptr) % 32 == 0) << std::endl;
    
    delete regular_ptr;
}

// =============================================================================
// 5. PERFORMANCE IMPLICATIONS
// =============================================================================

struct UnoptimizedStruct {
    bool flag1;     // 1 byte + 7 padding
    double value1;  // 8 bytes
    bool flag2;     // 1 byte + 7 padding
    double value2;  // 8 bytes
    bool flag3;     // 1 byte + 7 padding
    double value3;  // 8 bytes
};

struct OptimizedStruct {
    double value1;  // 8 bytes
    double value2;  // 8 bytes
    double value3;  // 8 bytes
    bool flag1;     // 1 byte
    bool flag2;     // 1 byte
    bool flag3;     // 1 byte + 5 padding
};

void demonstrate_performance_implications() {
    SECTION("Performance Implications");
    
    std::cout << "Memory usage comparison:" << std::endl;
    std::cout << "UnoptimizedStruct: " << sizeof(UnoptimizedStruct) << " bytes" << std::endl;
    std::cout << "OptimizedStruct: " << sizeof(OptimizedStruct) << " bytes" << std::endl;
    std::cout << "Memory saved: " << (sizeof(UnoptimizedStruct) - sizeof(OptimizedStruct)) << " bytes" << std::endl;
    
    // Cache line considerations
    std::cout << "\nCache line considerations:" << std::endl;
    std::cout << "Typical cache line size: 64 bytes" << std::endl;
    std::cout << "Objects per cache line:" << std::endl;
    std::cout << "UnoptimizedStruct: " << 64 / sizeof(UnoptimizedStruct) << std::endl;
    std::cout << "OptimizedStruct: " << 64 / sizeof(OptimizedStruct) << std::endl;
    
    // False sharing example
    struct PotentialFalseSharing {
        alignas(64) int counter1;  // On separate cache lines
        alignas(64) int counter2;
    };
    
    std::cout << "\nFalse sharing prevention:" << std::endl;
    std::cout << "PotentialFalseSharing size: " << sizeof(PotentialFalseSharing) << std::endl;
}

// =============================================================================
// 6. PRACTICAL EXAMPLES
// =============================================================================

// SIMD-friendly structure
struct alignas(16) Vec4 {
    float x, y, z, w;
    
    Vec4(float x = 0, float y = 0, float z = 0, float w = 0) 
        : x(x), y(y), z(z), w(w) {}
};

// Network packet structure
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t version;
    uint16_t type;
    uint32_t length;
    uint64_t timestamp;
};
#pragma pack(pop)

void demonstrate_practical_examples() {
    SECTION("Practical Examples");
    
    std::cout << "SIMD-aligned vector:" << std::endl;
    std::cout << "Vec4 size: " << sizeof(Vec4) << " bytes" << std::endl;
    std::cout << "Vec4 alignment: " << alignof(Vec4) << " bytes" << std::endl;
    
    Vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
    std::cout << "Vec4 address: " << &v << std::endl;
    std::cout << "Is 16-byte aligned: " << (reinterpret_cast<uintptr_t>(&v) % 16 == 0) << std::endl;
    
    std::cout << "\nNetwork packet (packed):" << std::endl;
    std::cout << "PacketHeader size: " << sizeof(PacketHeader) << " bytes" << std::endl;
    std::cout << "Expected size without packing: " << 
        sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint64_t) + 4 << " bytes" << std::endl;
    
    // Memory pool with aligned allocation
    constexpr size_t POOL_SIZE = 1024;
    constexpr size_t ALIGNMENT = 16;
    
    alignas(ALIGNMENT) char memory_pool[POOL_SIZE];
    std::cout << "\nMemory pool:" << std::endl;
    std::cout << "Pool address: " << (void*)memory_pool << std::endl;
    std::cout << "Is aligned: " << (reinterpret_cast<uintptr_t>(memory_pool) % ALIGNMENT == 0) << std::endl;
}

// =============================================================================
// 7. BEST PRACTICES
// =============================================================================

void demonstrate_best_practices() {
    SECTION("Best Practices");
    
    std::cout << "Memory alignment best practices:" << std::endl;
    std::cout << "1. Order struct members by size (largest first)" << std::endl;
    std::cout << "2. Group small members together" << std::endl;
    std::cout << "3. Use alignas for performance-critical structures" << std::endl;
    std::cout << "4. Consider cache line alignment for frequently accessed data" << std::endl;
    std::cout << "5. Use packed structures only when necessary (networking, file formats)" << std::endl;
    std::cout << "6. Be aware of false sharing in multi-threaded code" << std::endl;
    std::cout << "7. Use std::aligned_alloc for dynamic aligned allocation" << std::endl;
    
    // Example of good struct design
    struct WellDesignedStruct {
        // 8-byte members first
        double timestamp;
        void* ptr;
        
        // 4-byte members
        int id;
        float value;
        
        // 2-byte members
        short status;
        short flags;
        
        // 1-byte members
        char type;
        bool active;
        // 2 bytes padding here automatically
    };
    
    std::cout << "\nWell-designed struct size: " << sizeof(WellDesignedStruct) << " bytes" << std::endl;
    std::cout << "Alignment: " << alignof(WellDesignedStruct) << " bytes" << std::endl;
}

int main() {
    std::cout << "MEMORY ALIGNMENT AND PADDING TUTORIAL\n";
    std::cout << "=====================================\n";
    
    demonstrate_basic_alignment();
    demonstrate_struct_padding();
    demonstrate_alignas_alignof();
    demonstrate_dynamic_alignment();
    demonstrate_performance_implications();
    demonstrate_practical_examples();
    demonstrate_best_practices();
    
    std::cout << "\nTutorial completed!\n";
    return 0;
}

/*
KEY CONCEPTS:
- Memory alignment ensures data is stored at addresses divisible by its size
- Struct padding adds bytes to maintain alignment requirements
- alignof() returns alignment requirement, alignas() specifies alignment
- Proper member ordering reduces struct size and improves cache performance
- Packed structures eliminate padding but may hurt performance
- Cache line alignment prevents false sharing in multi-threaded code
- SIMD operations often require 16-byte alignment
- std::aligned_alloc() for dynamic aligned allocation
*/
