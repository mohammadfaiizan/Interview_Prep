/*
 * DESIGN PATTERNS - BUILDER PATTERN
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra builder_pattern.cpp -o builder
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. CLASSIC BUILDER PATTERN
// =============================================================================

class Computer {
private:
    std::string cpu_;
    std::string gpu_;
    int ram_gb_;
    int storage_gb_;
    bool has_wifi_;
    
public:
    Computer(const std::string& cpu, const std::string& gpu, int ram, int storage, bool wifi)
        : cpu_(cpu), gpu_(gpu), ram_gb_(ram), storage_gb_(storage), has_wifi_(wifi) {}
    
    void display() const {
        std::cout << "Computer Configuration:" << std::endl;
        std::cout << "  CPU: " << cpu_ << std::endl;
        std::cout << "  GPU: " << gpu_ << std::endl;
        std::cout << "  RAM: " << ram_gb_ << "GB" << std::endl;
        std::cout << "  Storage: " << storage_gb_ << "GB" << std::endl;
        std::cout << "  WiFi: " << (has_wifi_ ? "Yes" : "No") << std::endl;
    }
};

class ComputerBuilder {
protected:
    std::string cpu_ = "Basic CPU";
    std::string gpu_ = "Integrated GPU";
    int ram_gb_ = 8;
    int storage_gb_ = 256;
    bool has_wifi_ = true;
    
public:
    virtual ~ComputerBuilder() = default;
    
    virtual ComputerBuilder& setCPU(const std::string& cpu) { cpu_ = cpu; return *this; }
    virtual ComputerBuilder& setGPU(const std::string& gpu) { gpu_ = gpu; return *this; }
    virtual ComputerBuilder& setRAM(int ram) { ram_gb_ = ram; return *this; }
    virtual ComputerBuilder& setStorage(int storage) { storage_gb_ = storage; return *this; }
    virtual ComputerBuilder& setWiFi(bool wifi) { has_wifi_ = wifi; return *this; }
    
    virtual std::unique_ptr<Computer> build() {
        return std::make_unique<Computer>(cpu_, gpu_, ram_gb_, storage_gb_, has_wifi_);
    }
};

class GamingComputerBuilder : public ComputerBuilder {
public:
    GamingComputerBuilder() {
        cpu_ = "Intel i9";
        gpu_ = "RTX 4080";
        ram_gb_ = 32;
        storage_gb_ = 1024;
        has_wifi_ = true;
    }
};

class OfficeComputerBuilder : public ComputerBuilder {
public:
    OfficeComputerBuilder() {
        cpu_ = "Intel i5";
        gpu_ = "Integrated";
        ram_gb_ = 16;
        storage_gb_ = 512;
        has_wifi_ = true;
    }
};

void demonstrate_classic_builder() {
    SECTION("Classic Builder Pattern");
    
    // Gaming computer
    GamingComputerBuilder gaming_builder;
    auto gaming_pc = gaming_builder.setRAM(64).setStorage(2048).build();
    gaming_pc->display();
    
    // Office computer
    OfficeComputerBuilder office_builder;
    auto office_pc = office_builder.setRAM(8).build();
    office_pc->display();
}

// =============================================================================
// 2. FLUENT BUILDER
// =============================================================================

class FluentComputerBuilder {
private:
    std::string cpu_ = "Default CPU";
    std::string gpu_ = "Default GPU";
    int ram_gb_ = 8;
    int storage_gb_ = 256;
    bool has_wifi_ = true;
    
public:
    FluentComputerBuilder& cpu(const std::string& cpu) { cpu_ = cpu; return *this; }
    FluentComputerBuilder& gpu(const std::string& gpu) { gpu_ = gpu; return *this; }
    FluentComputerBuilder& ram(int ram) { ram_gb_ = ram; return *this; }
    FluentComputerBuilder& storage(int storage) { storage_gb_ = storage; return *this; }
    FluentComputerBuilder& wifi(bool wifi) { has_wifi_ = wifi; return *this; }
    
    std::unique_ptr<Computer> build() {
        return std::make_unique<Computer>(cpu_, gpu_, ram_gb_, storage_gb_, has_wifi_);
    }
};

void demonstrate_fluent_builder() {
    SECTION("Fluent Builder");
    
    auto computer = FluentComputerBuilder()
        .cpu("AMD Ryzen 9")
        .gpu("RTX 4090")
        .ram(64)
        .storage(2048)
        .wifi(true)
        .build();
    
    computer->display();
}

// =============================================================================
// 3. STEP BUILDER
// =============================================================================

class StepBuilder {
public:
    class CPUStep;
    class GPUStep;
    class RAMStep;
    class BuildStep;
    
    class CPUStep {
    private:
        std::string cpu_;
    public:
        explicit CPUStep(const std::string& cpu) : cpu_(cpu) {}
        GPUStep gpu(const std::string& gpu) { return GPUStep(cpu_, gpu); }
    };
    
    class GPUStep {
    private:
        std::string cpu_, gpu_;
    public:
        GPUStep(const std::string& cpu, const std::string& gpu) : cpu_(cpu), gpu_(gpu) {}
        RAMStep ram(int ram) { return RAMStep(cpu_, gpu_, ram); }
    };
    
    class RAMStep {
    private:
        std::string cpu_, gpu_;
        int ram_;
    public:
        RAMStep(const std::string& cpu, const std::string& gpu, int ram) 
            : cpu_(cpu), gpu_(gpu), ram_(ram) {}
        BuildStep storage(int storage) { return BuildStep(cpu_, gpu_, ram_, storage); }
    };
    
    class BuildStep {
    private:
        std::string cpu_, gpu_;
        int ram_, storage_;
    public:
        BuildStep(const std::string& cpu, const std::string& gpu, int ram, int storage)
            : cpu_(cpu), gpu_(gpu), ram_(ram), storage_(storage) {}
        
        std::unique_ptr<Computer> build() {
            return std::make_unique<Computer>(cpu_, gpu_, ram_, storage_, true);
        }
    };
    
    static CPUStep cpu(const std::string& cpu) {
        return CPUStep(cpu);
    }
};

void demonstrate_step_builder() {
    SECTION("Step Builder");
    
    auto computer = StepBuilder::cpu("Intel i7")
        .gpu("RTX 3070")
        .ram(16)
        .storage(512)
        .build();
    
    computer->display();
}

// =============================================================================
// 4. MODERN BUILDER WITH LAMBDAS
// =============================================================================

template<typename T>
class ModernBuilder {
private:
    T object_;
    
public:
    template<typename Func>
    ModernBuilder& with(Func&& func) {
        func(object_);
        return *this;
    }
    
    T build() && {
        return std::move(object_);
    }
};

struct ModernComputer {
    std::string cpu = "Default CPU";
    std::string gpu = "Default GPU";
    int ram_gb = 8;
    int storage_gb = 256;
    bool has_wifi = true;
    
    void display() const {
        std::cout << "Modern Computer:" << std::endl;
        std::cout << "  CPU: " << cpu << std::endl;
        std::cout << "  GPU: " << gpu << std::endl;
        std::cout << "  RAM: " << ram_gb << "GB" << std::endl;
        std::cout << "  Storage: " << storage_gb << "GB" << std::endl;
        std::cout << "  WiFi: " << (has_wifi ? "Yes" : "No") << std::endl;
    }
};

void demonstrate_modern_builder() {
    SECTION("Modern Builder with Lambdas");
    
    auto computer = ModernBuilder<ModernComputer>{}
        .with([](auto& c) { c.cpu = "AMD Ryzen 7"; })
        .with([](auto& c) { c.gpu = "RTX 4070"; })
        .with([](auto& c) { c.ram_gb = 32; })
        .with([](auto& c) { c.storage_gb = 1024; })
        .build();
    
    computer.display();
}

// =============================================================================
// 5. AGGREGATE INITIALIZATION BUILDER
// =============================================================================

struct SimpleComputer {
    std::string cpu = "Default";
    std::string gpu = "Default";
    int ram = 8;
    int storage = 256;
    
    void display() const {
        std::cout << "Simple Computer: " << cpu << ", " << gpu 
                  << ", " << ram << "GB RAM, " << storage << "GB storage" << std::endl;
    }
};

auto make_computer() {
    return [](auto cpu, auto gpu, auto ram, auto storage) {
        return SimpleComputer{cpu, gpu, ram, storage};
    };
}

void demonstrate_aggregate_builder() {
    SECTION("Aggregate Initialization Builder");
    
    auto computer = make_computer()("Intel i5", "GTX 1660", 16, 512);
    computer.display();
    
    // C++20 designated initializers
    SimpleComputer computer2{
        .cpu = "AMD Ryzen 5",
        .gpu = "RX 6600",
        .ram = 16,
        .storage = 1024
    };
    computer2.display();
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("Performance Comparison");
    
    const int iterations = 1000000;
    
    // Direct construction
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        Computer computer("CPU", "GPU", 16, 512, true);
        volatile auto* ptr = &computer;  // Prevent optimization
        (void)ptr;
    }
    auto direct_time = std::chrono::high_resolution_clock::now() - start;
    
    // Builder pattern
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto computer = FluentComputerBuilder()
            .cpu("CPU")
            .gpu("GPU")
            .ram(16)
            .storage(512)
            .build();
        volatile auto* ptr = computer.get();
        (void)ptr;
    }
    auto builder_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "Direct construction: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(direct_time).count() 
              << " ms" << std::endl;
    std::cout << "Builder pattern: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(builder_time).count() 
              << " ms" << std::endl;
}

int main() {
    std::cout << "BUILDER PATTERN TUTORIAL\n";
    std::cout << "========================\n";
    
    demonstrate_classic_builder();
    demonstrate_fluent_builder();
    demonstrate_step_builder();
    demonstrate_modern_builder();
    demonstrate_aggregate_builder();
    performance_comparison();
    
    std::cout << "\nBuilder Pattern Benefits:" << std::endl;
    std::cout << "- Readable object construction" << std::endl;
    std::cout << "- Step-by-step validation" << std::endl;
    std::cout << "- Immutable objects" << std::endl;
    std::cout << "- Complex object creation" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Builder pattern separates construction from representation
- Fluent interface enables method chaining
- Step builder enforces construction order
- Modern builders use lambdas and templates
- Aggregate initialization simplifies syntax
- Performance overhead varies with implementation
- Choose based on complexity and requirements
- C++20 designated initializers provide native builder-like syntax
*/
