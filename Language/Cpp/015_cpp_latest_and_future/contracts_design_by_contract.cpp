/*
 * C++ LATEST AND FUTURE - CONTRACTS (DESIGN BY CONTRACT)
 * 
 * Demonstration of proposed contract features for future C++ standards.
 * Contracts include preconditions, postconditions, and assertions.
 * 
 * Compilation: g++ -std=c++23 -DCONTRACTS_ENABLED -Wall -Wextra contracts_design_by_contract.cpp -o contracts_demo
 */

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <cmath>

// =============================================================================
// CONTRACT SIMULATION MACROS (Proposed syntax)
// =============================================================================

#ifdef CONTRACTS_ENABLED
    #define PRE(condition, message) \
        do { \
            if (!(condition)) { \
                std::cout << "PRECONDITION FAILED: " << message << std::endl; \
                throw std::invalid_argument("Precondition violation: " message); \
            } \
        } while(0)
    
    #define POST(condition, message) \
        do { \
            if (!(condition)) { \
                std::cout << "POSTCONDITION FAILED: " << message << std::endl; \
                throw std::logic_error("Postcondition violation: " message); \
            } \
        } while(0)
    
    #define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cout << "ASSERTION FAILED: " << message << std::endl; \
                throw std::logic_error("Assertion violation: " message); \
            } \
        } while(0)
#else
    #define PRE(condition, message) ((void)0)
    #define POST(condition, message) ((void)0)
    #define ASSERT(condition, message) ((void)0)
#endif

// =============================================================================
// BASIC CONTRACT EXAMPLES
// =============================================================================

// Simple function with preconditions
double divide(double a, double b) {
    PRE(b != 0.0, "Divisor cannot be zero");
    
    double result = a / b;
    
    POST(std::isfinite(result), "Result must be finite");
    return result;
}

// Function with multiple preconditions
double sqrt_safe(double x) {
    PRE(x >= 0.0, "Input must be non-negative");
    PRE(std::isfinite(x), "Input must be finite");
    
    double result = std::sqrt(x);
    
    POST(result >= 0.0, "Square root must be non-negative");
    POST(std::abs(result * result - x) < 1e-10, "Result squared should equal input");
    
    return result;
}

// Array access with bounds checking
template<typename T>
class SafeArray {
private:
    std::vector<T> data_;
    
public:
    SafeArray(size_t size) : data_(size) {
        PRE(size > 0, "Array size must be positive");
    }
    
    T& at(size_t index) {
        PRE(index < data_.size(), "Index must be within bounds");
        
        T& result = data_[index];
        
        // Postcondition: returned reference is valid
        POST(&result >= &data_[0] && &result < &data_[0] + data_.size(), 
             "Returned reference must be within array bounds");
        
        return result;
    }
    
    const T& at(size_t index) const {
        PRE(index < data_.size(), "Index must be within bounds");
        return data_[index];
    }
    
    size_t size() const {
        return data_.size();
    }
    
    void resize(size_t new_size) {
        PRE(new_size > 0, "New size must be positive");
        
        size_t old_size = data_.size();
        data_.resize(new_size);
        
        POST(data_.size() == new_size, "Size must match requested size");
        
        if (new_size > old_size) {
            POST(data_.capacity() >= new_size, "Capacity must accommodate new size");
        }
    }
};

// =============================================================================
// CLASS INVARIANTS
// =============================================================================

class BankAccount {
private:
    std::string account_number_;
    double balance_;
    double credit_limit_;
    
    // Class invariant checker
    void check_invariant() const {
        ASSERT(!account_number_.empty(), "Account number cannot be empty");
        ASSERT(balance_ >= -credit_limit_, "Balance cannot exceed credit limit");
        ASSERT(credit_limit_ >= 0.0, "Credit limit must be non-negative");
    }
    
public:
    BankAccount(const std::string& account_number, double initial_balance = 0.0, 
                double credit_limit = 0.0) 
        : account_number_(account_number), balance_(initial_balance), credit_limit_(credit_limit) {
        
        PRE(!account_number.empty(), "Account number cannot be empty");
        PRE(credit_limit >= 0.0, "Credit limit must be non-negative");
        PRE(initial_balance >= -credit_limit, "Initial balance cannot exceed credit limit");
        
        check_invariant();
    }
    
    void deposit(double amount) {
        PRE(amount > 0.0, "Deposit amount must be positive");
        PRE(std::isfinite(amount), "Deposit amount must be finite");
        
        double old_balance = balance_;
        balance_ += amount;
        
        POST(balance_ == old_balance + amount, "Balance must increase by deposit amount");
        check_invariant();
    }
    
    void withdraw(double amount) {
        PRE(amount > 0.0, "Withdrawal amount must be positive");
        PRE(std::isfinite(amount), "Withdrawal amount must be finite");
        PRE(balance_ - amount >= -credit_limit_, "Withdrawal would exceed credit limit");
        
        double old_balance = balance_;
        balance_ -= amount;
        
        POST(balance_ == old_balance - amount, "Balance must decrease by withdrawal amount");
        check_invariant();
    }
    
    double get_balance() const {
        check_invariant();
        return balance_;
    }
    
    double get_available_credit() const {
        check_invariant();
        double available = credit_limit_ + balance_;
        
        POST(available >= 0.0, "Available credit must be non-negative");
        return available;
    }
    
    void set_credit_limit(double new_limit) {
        PRE(new_limit >= 0.0, "Credit limit must be non-negative");
        PRE(balance_ >= -new_limit, "Current balance must not exceed new credit limit");
        
        credit_limit_ = new_limit;
        
        POST(credit_limit_ == new_limit, "Credit limit must be updated");
        check_invariant();
    }
};

// =============================================================================
// LOOP INVARIANTS AND COMPLEX CONTRACTS
// =============================================================================

// Binary search with loop invariants
template<typename T>
int binary_search(const std::vector<T>& arr, const T& target) {
    PRE(!arr.empty(), "Array cannot be empty");
    PRE(std::is_sorted(arr.begin(), arr.end()), "Array must be sorted");
    
    int left = 0;
    int right = static_cast<int>(arr.size()) - 1;
    
    while (left <= right) {
        // Loop invariant: if target exists, it's in range [left, right]
        ASSERT(left >= 0 && right < static_cast<int>(arr.size()), 
               "Indices must be within array bounds");
        
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            POST(arr[mid] == target, "Found element must equal target");
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    POST(left > right, "Search completed when left > right");
    return -1; // Not found
}

// Sorting with contracts
template<typename T>
void bubble_sort(std::vector<T>& arr) {
    if (arr.empty()) return;
    
    size_t n = arr.size();
    
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = 0; j < n - i - 1; ++j) {
            ASSERT(j < arr.size() && j + 1 < arr.size(), "Indices must be valid");
            
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
        }
        
        // Loop invariant: elements [n-i-1, n) are in their final sorted positions
        for (size_t k = n - i - 1; k < n - 1; ++k) {
            ASSERT(arr[k] <= arr[k + 1], "Suffix must be sorted");
        }
    }
    
    POST(std::is_sorted(arr.begin(), arr.end()), "Array must be sorted");
}

// =============================================================================
// DEMONSTRATION FUNCTIONS
// =============================================================================

void demonstrate_basic_contracts() {
    std::cout << "\n=== Basic Contract Examples ===" << std::endl;
    
    try {
        std::cout << "Testing divide function:" << std::endl;
        std::cout << "divide(10, 2) = " << divide(10, 2) << std::endl;
        std::cout << "divide(7, 3) = " << divide(7, 3) << std::endl;
        
        std::cout << "\nTesting sqrt_safe function:" << std::endl;
        std::cout << "sqrt_safe(16) = " << sqrt_safe(16) << std::endl;
        std::cout << "sqrt_safe(2) = " << sqrt_safe(2) << std::endl;
        
        // This should fail
        std::cout << "\nTesting contract violation:" << std::endl;
        divide(5, 0); // Should throw
        
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
}

void demonstrate_array_contracts() {
    std::cout << "\n=== Array Contract Examples ===" << std::endl;
    
    try {
        SafeArray<int> arr(5);
        
        std::cout << "Array size: " << arr.size() << std::endl;
        
        // Valid operations
        arr.at(0) = 10;
        arr.at(4) = 50;
        std::cout << "arr[0] = " << arr.at(0) << std::endl;
        std::cout << "arr[4] = " << arr.at(4) << std::endl;
        
        // This should fail
        std::cout << "\nTesting bounds violation:" << std::endl;
        arr.at(10) = 100; // Should throw
        
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
}

void demonstrate_class_invariants() {
    std::cout << "\n=== Class Invariant Examples ===" << std::endl;
    
    try {
        BankAccount account("ACC123", 1000.0, 500.0);
        
        std::cout << "Initial balance: $" << account.get_balance() << std::endl;
        std::cout << "Available credit: $" << account.get_available_credit() << std::endl;
        
        account.deposit(200.0);
        std::cout << "After deposit: $" << account.get_balance() << std::endl;
        
        account.withdraw(300.0);
        std::cout << "After withdrawal: $" << account.get_balance() << std::endl;
        
        // This should fail
        std::cout << "\nTesting overdraft limit:" << std::endl;
        account.withdraw(2000.0); // Should throw
        
    } catch (const std::exception& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
}

void demonstrate_algorithm_contracts() {
    std::cout << "\n=== Algorithm Contract Examples ===" << std::endl;
    
    std::vector<int> sorted_data = {1, 3, 5, 7, 9, 11, 13, 15};
    
    std::cout << "Binary search in: ";
    for (int x : sorted_data) std::cout << x << " ";
    std::cout << std::endl;
    
    int index = binary_search(sorted_data, 7);
    std::cout << "Found 7 at index: " << index << std::endl;
    
    index = binary_search(sorted_data, 4);
    std::cout << "Found 4 at index: " << index << " (not found)" << std::endl;
    
    std::vector<int> unsorted_data = {5, 2, 8, 1, 9};
    std::cout << "\nBefore sorting: ";
    for (int x : unsorted_data) std::cout << x << " ";
    std::cout << std::endl;
    
    bubble_sort(unsorted_data);
    std::cout << "After sorting: ";
    for (int x : unsorted_data) std::cout << x << " ";
    std::cout << std::endl;
}

void demonstrate_proposed_syntax() {
    std::cout << "\n=== Proposed Contract Syntax ===" << std::endl;
    
    std::cout << "Future C++ contract syntax:" << std::endl;
    std::cout << std::endl;
    
    std::cout << "double divide(double a, double b)" << std::endl;
    std::cout << "  pre: b != 0.0" << std::endl;
    std::cout << "  post r: std::isfinite(r)" << std::endl;
    std::cout << "{" << std::endl;
    std::cout << "    return a / b;" << std::endl;
    std::cout << "}" << std::endl;
    std::cout << std::endl;
    
    std::cout << "class BankAccount {" << std::endl;
    std::cout << "    invariant: balance_ >= -credit_limit_;" << std::endl;
    std::cout << "    invariant: !account_number_.empty();" << std::endl;
    std::cout << std::endl;
    std::cout << "    void withdraw(double amount)" << std::endl;
    std::cout << "      pre: amount > 0.0" << std::endl;
    std::cout << "      pre: balance_ - amount >= -credit_limit_" << std::endl;
    std::cout << "      post: balance_ == old(balance_) - amount" << std::endl;
    std::cout << "    { /* implementation */ }" << std::endl;
    std::cout << "};" << std::endl;
}

int main() {
    std::cout << "C++ CONTRACTS (DESIGN BY CONTRACT)\n";
    std::cout << "==================================\n";
    
    try {
        demonstrate_basic_contracts();
        demonstrate_array_contracts();
        demonstrate_class_invariants();
        demonstrate_algorithm_contracts();
        demonstrate_proposed_syntax();
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nContract Benefits:" << std::endl;
    std::cout << "• Documentation in code" << std::endl;
    std::cout << "• Automatic testing" << std::endl;
    std::cout << "• Debugging assistance" << std::endl;
    std::cout << "• Optimization opportunities" << std::endl;
    
    return 0;
}
