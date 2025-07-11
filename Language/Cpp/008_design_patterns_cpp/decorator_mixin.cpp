/*
 * DESIGN PATTERNS - DECORATOR AND MIXIN PATTERNS
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra decorator_mixin.cpp -o decorator
 */

#include <iostream>
#include <string>
#include <memory>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. CLASSIC DECORATOR PATTERN
// =============================================================================

class Coffee {
public:
    virtual ~Coffee() = default;
    virtual std::string getDescription() const = 0;
    virtual double getCost() const = 0;
};

class SimpleCoffee : public Coffee {
public:
    std::string getDescription() const override {
        return "Simple coffee";
    }
    
    double getCost() const override {
        return 2.0;
    }
};

class CoffeeDecorator : public Coffee {
protected:
    std::unique_ptr<Coffee> coffee_;
    
public:
    explicit CoffeeDecorator(std::unique_ptr<Coffee> coffee) 
        : coffee_(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription();
    }
    
    double getCost() const override {
        return coffee_->getCost();
    }
};

class MilkDecorator : public CoffeeDecorator {
public:
    explicit MilkDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", milk";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.5;
    }
};

class SugarDecorator : public CoffeeDecorator {
public:
    explicit SugarDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", sugar";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.2;
    }
};

void demonstrate_classic_decorator() {
    SECTION("Classic Decorator Pattern");
    
    // Simple coffee
    auto coffee = std::make_unique<SimpleCoffee>();
    std::cout << coffee->getDescription() << " - $" << coffee->getCost() << std::endl;
    
    // Coffee with milk
    auto coffee_with_milk = std::make_unique<MilkDecorator>(std::move(coffee));
    std::cout << coffee_with_milk->getDescription() << " - $" << coffee_with_milk->getCost() << std::endl;
    
    // Coffee with milk and sugar
    auto fancy_coffee = std::make_unique<SugarDecorator>(std::move(coffee_with_milk));
    std::cout << fancy_coffee->getDescription() << " - $" << fancy_coffee->getCost() << std::endl;
}

// =============================================================================
// 2. TEMPLATE-BASED DECORATOR
// =============================================================================

template<typename Base>
class LoggingDecorator : public Base {
public:
    template<typename... Args>
    LoggingDecorator(Args&&... args) : Base(std::forward<Args>(args)...) {}
    
    void execute() {
        std::cout << "[LOG] Before execution" << std::endl;
        Base::execute();
        std::cout << "[LOG] After execution" << std::endl;
    }
};

template<typename Base>
class TimingDecorator : public Base {
public:
    template<typename... Args>
    TimingDecorator(Args&&... args) : Base(std::forward<Args>(args)...) {}
    
    void execute() {
        auto start = std::chrono::high_resolution_clock::now();
        Base::execute();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "[TIMING] Execution took " << duration.count() << " μs" << std::endl;
    }
};

class SimpleTask {
public:
    void execute() {
        std::cout << "Executing simple task" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
};

void demonstrate_template_decorator() {
    SECTION("Template-based Decorator");
    
    // Simple task
    SimpleTask simple_task;
    simple_task.execute();
    
    std::cout << "\nWith logging:" << std::endl;
    LoggingDecorator<SimpleTask> logged_task;
    logged_task.execute();
    
    std::cout << "\nWith timing:" << std::endl;
    TimingDecorator<SimpleTask> timed_task;
    timed_task.execute();
    
    std::cout << "\nWith both logging and timing:" << std::endl;
    TimingDecorator<LoggingDecorator<SimpleTask>> decorated_task;
    decorated_task.execute();
}

// =============================================================================
// 3. MIXIN PATTERN
// =============================================================================

template<typename Derived>
class Printable {
public:
    void print() const {
        std::cout << "Printing: " << static_cast<const Derived&>(*this).toString() << std::endl;
    }
};

template<typename Derived>
class Serializable {
public:
    std::string serialize() const {
        return "Serialized: " + static_cast<const Derived&>(*this).toString();
    }
};

template<typename Derived>
class Comparable {
public:
    bool equals(const Derived& other) const {
        return static_cast<const Derived&>(*this).toString() == other.toString();
    }
};

class Person : public Printable<Person>, 
               public Serializable<Person>, 
               public Comparable<Person> {
private:
    std::string name_;
    int age_;
    
public:
    Person(const std::string& name, int age) : name_(name), age_(age) {}
    
    std::string toString() const {
        return name_ + " (" + std::to_string(age_) + ")";
    }
    
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
};

void demonstrate_mixin_pattern() {
    SECTION("Mixin Pattern");
    
    Person person1("Alice", 30);
    Person person2("Bob", 25);
    Person person3("Alice", 30);
    
    // Using mixin functionality
    person1.print();
    std::cout << person1.serialize() << std::endl;
    
    std::cout << "person1 equals person2: " << person1.equals(person2) << std::endl;
    std::cout << "person1 equals person3: " << person1.equals(person3) << std::endl;
}

// =============================================================================
// 4. POLICY-BASED MIXINS
// =============================================================================

template<typename T>
class VectorStorage {
private:
    std::vector<T> data_;
    
public:
    void add(const T& item) { data_.push_back(item); }
    T get(size_t index) const { return data_[index]; }
    size_t size() const { return data_.size(); }
};

template<typename T>
class ListStorage {
private:
    std::list<T> data_;
    
public:
    void add(const T& item) { data_.push_back(item); }
    T get(size_t index) const {
        auto it = data_.begin();
        std::advance(it, index);
        return *it;
    }
    size_t size() const { return data_.size(); }
};

template<typename Derived>
class Iterable {
public:
    void forEach(std::function<void(const auto&)> func) const {
        const auto& derived = static_cast<const Derived&>(*this);
        for (size_t i = 0; i < derived.size(); ++i) {
            func(derived.get(i));
        }
    }
};

template<typename T, template<typename> class Storage>
class Container : public Storage<T>, public Iterable<Container<T, Storage>> {
public:
    using Storage<T>::add;
    using Storage<T>::get;
    using Storage<T>::size;
};

void demonstrate_policy_mixins() {
    SECTION("Policy-based Mixins");
    
    Container<int, VectorStorage> vector_container;
    vector_container.add(1);
    vector_container.add(2);
    vector_container.add(3);
    
    std::cout << "Vector container contents: ";
    vector_container.forEach([](const auto& item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;
    
    Container<std::string, ListStorage> list_container;
    list_container.add("hello");
    list_container.add("world");
    
    std::cout << "List container contents: ";
    list_container.forEach([](const auto& item) {
        std::cout << item << " ";
    });
    std::cout << std::endl;
}

// =============================================================================
// 5. CRTP MIXINS
// =============================================================================

template<typename Derived>
class Cloneable {
public:
    std::unique_ptr<Derived> clone() const {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

template<typename Derived>
class Countable {
private:
    static inline int count_ = 0;
    
public:
    Countable() { ++count_; }
    Countable(const Countable&) { ++count_; }
    ~Countable() { --count_; }
    
    static int getCount() { return count_; }
};

class Shape : public Cloneable<Shape>, public Countable<Shape> {
private:
    std::string type_;
    
public:
    explicit Shape(const std::string& type) : type_(type) {}
    
    const std::string& getType() const { return type_; }
    
    void display() const {
        std::cout << "Shape: " << type_ << std::endl;
    }
};

void demonstrate_crtp_mixins() {
    SECTION("CRTP Mixins");
    
    std::cout << "Initial count: " << Shape::getCount() << std::endl;
    
    {
        Shape circle("Circle");
        Shape square("Square");
        
        std::cout << "After creating 2 shapes: " << Shape::getCount() << std::endl;
        
        auto cloned_circle = circle.clone();
        cloned_circle->display();
        
        std::cout << "After cloning: " << Shape::getCount() << std::endl;
    }
    
    std::cout << "After scope exit: " << Shape::getCount() << std::endl;
}

// =============================================================================
// 6. FUNCTIONAL DECORATORS
// =============================================================================

template<typename Func>
auto add_logging(Func&& func) {
    return [func = std::forward<Func>(func)](auto&&... args) {
        std::cout << "[LOG] Function called" << std::endl;
        auto result = func(std::forward<decltype(args)>(args)...);
        std::cout << "[LOG] Function completed" << std::endl;
        return result;
    };
}

template<typename Func>
auto add_timing(Func&& func) {
    return [func = std::forward<Func>(func)](auto&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func(std::forward<decltype(args)>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "[TIMING] Function took " << duration.count() << " μs" << std::endl;
        return result;
    };
}

int expensive_calculation(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return n * n;
}

void demonstrate_functional_decorators() {
    SECTION("Functional Decorators");
    
    // Original function
    int result1 = expensive_calculation(5);
    std::cout << "Result: " << result1 << std::endl;
    
    // With logging
    auto logged_func = add_logging(expensive_calculation);
    int result2 = logged_func(5);
    std::cout << "Result: " << result2 << std::endl;
    
    // With timing and logging
    auto decorated_func = add_timing(add_logging(expensive_calculation));
    int result3 = decorated_func(5);
    std::cout << "Result: " << result3 << std::endl;
}

int main() {
    std::cout << "DECORATOR AND MIXIN PATTERNS\n";
    std::cout << "============================\n";
    
    demonstrate_classic_decorator();
    demonstrate_template_decorator();
    demonstrate_mixin_pattern();
    demonstrate_policy_mixins();
    demonstrate_crtp_mixins();
    demonstrate_functional_decorators();
    
    std::cout << "\nPattern Comparison:" << std::endl;
    std::cout << "Classic Decorator: Runtime composition, virtual functions" << std::endl;
    std::cout << "Template Decorator: Compile-time composition, zero overhead" << std::endl;
    std::cout << "Mixins: Multiple inheritance, shared functionality" << std::endl;
    std::cout << "CRTP Mixins: Static polymorphism, type-safe" << std::endl;
    std::cout << "Functional: Lambda-based, composable" << std::endl;
    
    return 0;
}

/*
KEY CONCEPTS:
- Decorator adds behavior without modifying structure
- Template decorators provide zero-overhead composition
- Mixins enable multiple inheritance of functionality
- CRTP mixins provide type-safe static polymorphism
- Policy-based design combines orthogonal concerns
- Functional decorators use lambdas for composition
- Choose based on performance and flexibility requirements
- Modern C++ favors templates over virtual inheritance
*/
