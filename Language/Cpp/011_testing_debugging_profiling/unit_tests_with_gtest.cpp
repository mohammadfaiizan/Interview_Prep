/*
 * TESTING, DEBUGGING, PROFILING - UNIT TESTS WITH GOOGLE TEST
 * 
 * This file demonstrates comprehensive unit testing using Google Test (gtest)
 * framework with practical examples and best practices.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra unit_tests_with_gtest.cpp -lgtest -lgtest_main -pthread -o test_runner
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>

// Note: In real projects, include <gtest/gtest.h>
// For this demonstration, we'll simulate the gtest interface

// =============================================================================
// CLASSES TO TEST - EXAMPLE IMPLEMENTATIONS
// =============================================================================

// Simple calculator class for basic testing
class Calculator {
public:
    double add(double a, double b) const { return a + b; }
    double subtract(double a, double b) const { return a - b; }
    double multiply(double a, double b) const { return a * b; }
    
    double divide(double a, double b) const {
        if (b == 0.0) {
            throw std::invalid_argument("Division by zero");
        }
        return a / b;
    }
    
    double power(double base, int exponent) const {
        return std::pow(base, exponent);
    }
    
    double sqrt_positive(double value) const {
        if (value < 0) {
            throw std::invalid_argument("Cannot compute square root of negative number");
        }
        return std::sqrt(value);
    }
};

// String utility class for testing
class StringUtils {
public:
    static std::string to_upper(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            c = std::toupper(c);
        }
        return result;
    }
    
    static std::string reverse(const std::string& str) {
        std::string result = str;
        std::reverse(result.begin(), result.end());
        return result;
    }
    
    static bool is_palindrome(const std::string& str) {
        std::string clean = str;
        clean.erase(std::remove_if(clean.begin(), clean.end(), 
                   [](char c) { return !std::isalnum(c); }), clean.end());
        std::transform(clean.begin(), clean.end(), clean.begin(), ::tolower);
        return clean == reverse(clean);
    }
    
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : str) {
            if (c == delimiter) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

// Bank account class for state-based testing
class BankAccount {
private:
    std::string account_number_;
    double balance_;
    bool is_frozen_;
    
public:
    BankAccount(const std::string& account_number, double initial_balance = 0.0)
        : account_number_(account_number), balance_(initial_balance), is_frozen_(false) {
        if (initial_balance < 0) {
            throw std::invalid_argument("Initial balance cannot be negative");
        }
    }
    
    void deposit(double amount) {
        if (is_frozen_) {
            throw std::runtime_error("Account is frozen");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive");
        }
        balance_ += amount;
    }
    
    void withdraw(double amount) {
        if (is_frozen_) {
            throw std::runtime_error("Account is frozen");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Withdrawal amount must be positive");
        }
        if (amount > balance_) {
            throw std::runtime_error("Insufficient funds");
        }
        balance_ -= amount;
    }
    
    double get_balance() const { return balance_; }
    const std::string& get_account_number() const { return account_number_; }
    bool is_frozen() const { return is_frozen_; }
    
    void freeze() { is_frozen_ = true; }
    void unfreeze() { is_frozen_ = false; }
};

// =============================================================================
// MOCK GTEST INTERFACE FOR DEMONSTRATION
// =============================================================================

namespace testing {
    class Test {
    public:
        virtual ~Test() = default;
        virtual void SetUp() {}
        virtual void TearDown() {}
        static void run_test(const std::string& test_name, std::function<void()> test_func) {
            std::cout << "[ RUN      ] " << test_name << std::endl;
            try {
                test_func();
                std::cout << "[       OK ] " << test_name << std::endl;
            } catch (const std::exception& e) {
                std::cout << "[  FAILED  ] " << test_name << " - " << e.what() << std::endl;
            }
        }
    };
}

#define TEST(test_case_name, test_name) \
    void test_case_name##_##test_name(); \
    struct test_case_name##_##test_name##_runner { \
        test_case_name##_##test_name##_runner() { \
            testing::Test::run_test(#test_case_name "." #test_name, test_case_name##_##test_name); \
        } \
    }; \
    static test_case_name##_##test_name##_runner test_case_name##_##test_name##_instance; \
    void test_case_name##_##test_name()

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Expected " + std::to_string(expected) + " but got " + std::to_string(actual)); \
    }

#define EXPECT_DOUBLE_EQ(expected, actual) \
    if (std::abs((expected) - (actual)) > 1e-9) { \
        throw std::runtime_error("Expected " + std::to_string(expected) + " but got " + std::to_string(actual)); \
    }

#define EXPECT_STREQ(expected, actual) \
    if (std::string(expected) != std::string(actual)) { \
        throw std::runtime_error("Expected '" + std::string(expected) + "' but got '" + std::string(actual) + "'"); \
    }

#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Expected true but got false"); \
    }

#define EXPECT_FALSE(condition) \
    if (condition) { \
        throw std::runtime_error("Expected false but got true"); \
    }

#define EXPECT_THROW(statement, exception_type) \
    try { \
        statement; \
        throw std::runtime_error("Expected exception but none was thrown"); \
    } catch (const exception_type&) { \
        /* Expected exception caught */ \
    }

// =============================================================================
// BASIC UNIT TESTS
// =============================================================================

TEST(CalculatorTest, BasicArithmetic) {
    Calculator calc;
    
    EXPECT_DOUBLE_EQ(5.0, calc.add(2.0, 3.0));
    EXPECT_DOUBLE_EQ(-1.0, calc.subtract(2.0, 3.0));
    EXPECT_DOUBLE_EQ(6.0, calc.multiply(2.0, 3.0));
    EXPECT_DOUBLE_EQ(2.0, calc.divide(6.0, 3.0));
}

TEST(CalculatorTest, EdgeCases) {
    Calculator calc;
    
    // Test with zero
    EXPECT_DOUBLE_EQ(5.0, calc.add(5.0, 0.0));
    EXPECT_DOUBLE_EQ(0.0, calc.multiply(5.0, 0.0));
    
    // Test with negative numbers
    EXPECT_DOUBLE_EQ(-2.0, calc.add(-5.0, 3.0));
    EXPECT_DOUBLE_EQ(15.0, calc.multiply(-5.0, -3.0));
}

TEST(CalculatorTest, ExceptionHandling) {
    Calculator calc;
    
    // Test division by zero
    EXPECT_THROW(calc.divide(5.0, 0.0), std::invalid_argument);
    
    // Test square root of negative number
    EXPECT_THROW(calc.sqrt_positive(-1.0), std::invalid_argument);
}

TEST(CalculatorTest, PowerFunction) {
    Calculator calc;
    
    EXPECT_DOUBLE_EQ(8.0, calc.power(2.0, 3));
    EXPECT_DOUBLE_EQ(1.0, calc.power(5.0, 0));
    EXPECT_DOUBLE_EQ(0.25, calc.power(2.0, -2));
}

// =============================================================================
// STRING UTILITY TESTS
// =============================================================================

TEST(StringUtilsTest, ToUpper) {
    EXPECT_STREQ("HELLO", StringUtils::to_upper("hello").c_str());
    EXPECT_STREQ("HELLO WORLD", StringUtils::to_upper("Hello World").c_str());
    EXPECT_STREQ("123ABC", StringUtils::to_upper("123abc").c_str());
    EXPECT_STREQ("", StringUtils::to_upper("").c_str());
}

TEST(StringUtilsTest, Reverse) {
    EXPECT_STREQ("olleh", StringUtils::reverse("hello").c_str());
    EXPECT_STREQ("dlrow", StringUtils::reverse("world").c_str());
    EXPECT_STREQ("", StringUtils::reverse("").c_str());
    EXPECT_STREQ("a", StringUtils::reverse("a").c_str());
}

TEST(StringUtilsTest, Palindrome) {
    EXPECT_TRUE(StringUtils::is_palindrome("racecar"));
    EXPECT_TRUE(StringUtils::is_palindrome("A man a plan a canal Panama"));
    EXPECT_TRUE(StringUtils::is_palindrome("race a car")); // This should actually be false
    EXPECT_FALSE(StringUtils::is_palindrome("hello"));
    EXPECT_TRUE(StringUtils::is_palindrome(""));
    EXPECT_TRUE(StringUtils::is_palindrome("a"));
}

TEST(StringUtilsTest, Split) {
    auto result = StringUtils::split("hello,world,test", ',');
    EXPECT_EQ(3, result.size());
    EXPECT_STREQ("hello", result[0].c_str());
    EXPECT_STREQ("world", result[1].c_str());
    EXPECT_STREQ("test", result[2].c_str());
    
    // Test empty string
    auto empty_result = StringUtils::split("", ',');
    EXPECT_EQ(0, empty_result.size());
    
    // Test no delimiter
    auto no_delim = StringUtils::split("hello", ',');
    EXPECT_EQ(1, no_delim.size());
    EXPECT_STREQ("hello", no_delim[0].c_str());
}

// =============================================================================
// STATE-BASED TESTING
// =============================================================================

TEST(BankAccountTest, Construction) {
    BankAccount account("12345", 100.0);
    EXPECT_STREQ("12345", account.get_account_number().c_str());
    EXPECT_DOUBLE_EQ(100.0, account.get_balance());
    EXPECT_FALSE(account.is_frozen());
}

TEST(BankAccountTest, ConstructionWithNegativeBalance) {
    EXPECT_THROW(BankAccount("12345", -100.0), std::invalid_argument);
}

TEST(BankAccountTest, Deposit) {
    BankAccount account("12345", 100.0);
    
    account.deposit(50.0);
    EXPECT_DOUBLE_EQ(150.0, account.get_balance());
    
    account.deposit(25.0);
    EXPECT_DOUBLE_EQ(175.0, account.get_balance());
}

TEST(BankAccountTest, DepositInvalidAmount) {
    BankAccount account("12345", 100.0);
    
    EXPECT_THROW(account.deposit(0.0), std::invalid_argument);
    EXPECT_THROW(account.deposit(-50.0), std::invalid_argument);
}

TEST(BankAccountTest, Withdrawal) {
    BankAccount account("12345", 100.0);
    
    account.withdraw(30.0);
    EXPECT_DOUBLE_EQ(70.0, account.get_balance());
    
    account.withdraw(70.0);
    EXPECT_DOUBLE_EQ(0.0, account.get_balance());
}

TEST(BankAccountTest, WithdrawalInsufficientFunds) {
    BankAccount account("12345", 50.0);
    
    EXPECT_THROW(account.withdraw(100.0), std::runtime_error);
    EXPECT_DOUBLE_EQ(50.0, account.get_balance()); // Balance should remain unchanged
}

TEST(BankAccountTest, FrozenAccount) {
    BankAccount account("12345", 100.0);
    
    account.freeze();
    EXPECT_TRUE(account.is_frozen());
    
    EXPECT_THROW(account.deposit(50.0), std::runtime_error);
    EXPECT_THROW(account.withdraw(30.0), std::runtime_error);
    
    account.unfreeze();
    EXPECT_FALSE(account.is_frozen());
    
    // Should work after unfreezing
    account.deposit(50.0);
    EXPECT_DOUBLE_EQ(150.0, account.get_balance());
}

// =============================================================================
// GTEST FEATURES DEMONSTRATION
// =============================================================================

void demonstrate_gtest_features() {
    std::cout << "\n=== Google Test Features ===" << std::endl;
    
    std::cout << "\n1. Basic Assertions:" << std::endl;
    std::cout << R"(
EXPECT_EQ(expected, actual)     // Equality
EXPECT_NE(val1, val2)          // Inequality
EXPECT_LT(val1, val2)          // Less than
EXPECT_LE(val1, val2)          // Less than or equal
EXPECT_GT(val1, val2)          // Greater than
EXPECT_GE(val1, val2)          // Greater than or equal

EXPECT_TRUE(condition)          // Boolean true
EXPECT_FALSE(condition)         // Boolean false

EXPECT_STREQ(str1, str2)       // String equality (C strings)
EXPECT_STRNE(str1, str2)       // String inequality
)" << std::endl;

    std::cout << "\n2. Floating Point Assertions:" << std::endl;
    std::cout << R"(
EXPECT_FLOAT_EQ(expected, actual)   // Float equality with tolerance
EXPECT_DOUBLE_EQ(expected, actual)  // Double equality with tolerance
EXPECT_NEAR(val1, val2, tolerance)  // Near equality with custom tolerance
)" << std::endl;

    std::cout << "\n3. Exception Assertions:" << std::endl;
    std::cout << R"(
EXPECT_THROW(statement, exception_type)     // Expects specific exception
EXPECT_ANY_THROW(statement)                 // Expects any exception
EXPECT_NO_THROW(statement)                  // Expects no exception
)" << std::endl;

    std::cout << "\n4. Fatal vs Non-Fatal Assertions:" << std::endl;
    std::cout << R"(
// Non-fatal (EXPECT_*): Test continues even if assertion fails
EXPECT_EQ(2, add(1, 1));
EXPECT_EQ(4, add(2, 2));  // This will still run even if above fails

// Fatal (ASSERT_*): Test stops if assertion fails
ASSERT_EQ(2, add(1, 1));
EXPECT_EQ(4, add(2, 2));  // This won't run if ASSERT above fails
)" << std::endl;

    std::cout << "\n5. Test Fixtures:" << std::endl;
    std::cout << R"(
class CalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        calc = std::make_unique<Calculator>();
    }
    
    void TearDown() override {
        calc.reset();
    }
    
    std::unique_ptr<Calculator> calc;
};

TEST_F(CalculatorTest, Addition) {
    EXPECT_EQ(5, calc->add(2, 3));
}
)" << std::endl;

    std::cout << "\n6. Parameterized Tests:" << std::endl;
    std::cout << R"(
class ParameterizedTest : public ::testing::TestWithParam<int> {};

TEST_P(ParameterizedTest, IsEven) {
    int value = GetParam();
    EXPECT_EQ(0, value % 2);
}

INSTANTIATE_TEST_SUITE_P(EvenNumbers, ParameterizedTest,
                        ::testing::Values(2, 4, 6, 8, 10));
)" << std::endl;
}

void demonstrate_test_organization() {
    std::cout << "\n=== Test Organization Best Practices ===" << std::endl;
    
    std::cout << "\n1. Test Naming Convention:" << std::endl;
    std::cout << "   • TestSuiteName_TestName format" << std::endl;
    std::cout << "   • Descriptive names explaining what is tested" << std::endl;
    std::cout << "   • Group related tests in same test suite" << std::endl;
    
    std::cout << "\n2. Test Structure (AAA Pattern):" << std::endl;
    std::cout << R"(
TEST(CalculatorTest, Addition) {
    // Arrange
    Calculator calc;
    double a = 2.0, b = 3.0;
    
    // Act
    double result = calc.add(a, b);
    
    // Assert
    EXPECT_EQ(5.0, result);
}
)" << std::endl;

    std::cout << "\n3. Test Categories:" << std::endl;
    std::cout << "   • Unit Tests: Test individual functions/methods" << std::endl;
    std::cout << "   • Integration Tests: Test component interactions" << std::endl;
    std::cout << "   • Edge Case Tests: Test boundary conditions" << std::endl;
    std::cout << "   • Error Handling Tests: Test exception scenarios" << std::endl;
    
    std::cout << "\n4. CMakeLists.txt for Google Test:" << std::endl;
    std::cout << R"(
# Find Google Test
find_package(GTest REQUIRED)

# Create test executable
add_executable(unit_tests
    test_calculator.cpp
    test_string_utils.cpp
    test_bank_account.cpp)

# Link Google Test
target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        my_library_to_test)

# Enable testing
enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)

# Add custom target for running tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS unit_tests)
)" << std::endl;
}

void demonstrate_advanced_testing() {
    std::cout << "\n=== Advanced Testing Techniques ===" << std::endl;
    
    std::cout << "\n1. Death Tests (Testing Crashes):" << std::endl;
    std::cout << R"(
TEST(DeathTest, DivisionByZero) {
    EXPECT_DEATH(divide_and_crash(1, 0), "Division by zero");
}

// For debug builds only
#ifndef NDEBUG
TEST(DeathTest, AssertionFailure) {
    EXPECT_DEBUG_DEATH(assert(false), "Assertion.*failed");
}
#endif
)" << std::endl;

    std::cout << "\n2. Custom Matchers:" << std::endl;
    std::cout << R"(
MATCHER_P(IsNear, expected, "") {
    return std::abs(arg - expected) < 0.01;
}

TEST(CustomMatcherTest, FloatingPoint) {
    EXPECT_THAT(3.14159, IsNear(3.14));
}
)" << std::endl;

    std::cout << "\n3. Test Listeners:" << std::endl;
    std::cout << R"(
class CustomTestListener : public ::testing::TestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cout << "Starting test: " << test_info.name() << std::endl;
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        std::cout << "Finished test: " << test_info.name() << std::endl;
    }
};

// In main()
::testing::TestEventListeners& listeners = 
    ::testing::UnitTest::GetInstance()->listeners();
listeners.Append(new CustomTestListener);
)" << std::endl;

    std::cout << "\n4. Performance Testing:" << std::endl;
    std::cout << R"(
TEST(PerformanceTest, SortingAlgorithm) {
    std::vector<int> data(10000);
    std::iota(data.rbegin(), data.rend(), 1);  // Reverse sorted
    
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 100);  // Should complete in < 100ms
}
)" << std::endl;
}

void demonstrate_test_coverage() {
    std::cout << "\n=== Test Coverage and Quality Metrics ===" << std::endl;
    
    std::cout << "\n1. Code Coverage with gcov:" << std::endl;
    std::cout << "# Compile with coverage flags" << std::endl;
    std::cout << "g++ -std=c++17 --coverage -O0 source.cpp test.cpp -lgtest -o test" << std::endl;
    std::cout << "./test" << std::endl;
    std::cout << "gcov source.cpp" << std::endl;
    std::cout << "lcov --capture --directory . --output-file coverage.info" << std::endl;
    std::cout << "genhtml coverage.info --output-directory coverage_report" << std::endl;
    
    std::cout << "\n2. CMake Coverage Configuration:" << std::endl;
    std::cout << R"(
# Add coverage option
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(my_target PRIVATE --coverage)
        target_link_options(my_target PRIVATE --coverage)
    endif()
endif()

# Custom target for coverage report
if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND lcov --capture --directory . --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report")
endif()
)" << std::endl;

    std::cout << "\n3. Coverage Metrics:" << std::endl;
    std::cout << "   • Line Coverage: % of code lines executed" << std::endl;
    std::cout << "   • Branch Coverage: % of decision branches taken" << std::endl;
    std::cout << "   • Function Coverage: % of functions called" << std::endl;
    std::cout << "   • Target: Aim for >80% line coverage, >70% branch coverage" << std::endl;
    
    std::cout << "\n4. Quality Guidelines:" << std::endl;
    std::cout << "   • Write tests before or alongside code (TDD)" << std::endl;
    std::cout << "   • Each test should test one specific behavior" << std::endl;
    std::cout << "   • Tests should be independent and repeatable" << std::endl;
    std::cout << "   • Use descriptive test names and error messages" << std::endl;
    std::cout << "   • Maintain tests as carefully as production code" << std::endl;
}

int main() {
    std::cout << "UNIT TESTING WITH GOOGLE TEST\n";
    std::cout << "==============================\n";
    
    std::cout << "Running demonstration tests...\n" << std::endl;
    
    // Tests are automatically run via static initialization
    // In real gtest, you would call RUN_ALL_TESTS()
    
    demonstrate_gtest_features();
    demonstrate_test_organization();
    demonstrate_advanced_testing();
    demonstrate_test_coverage();
    
    std::cout << "\nKey Testing Concepts:" << std::endl;
    std::cout << "• Unit tests verify individual components in isolation" << std::endl;
    std::cout << "• Google Test provides comprehensive assertion framework" << std::endl;
    std::cout << "• Test fixtures enable setup/teardown for complex tests" << std::endl;
    std::cout << "• Parameterized tests reduce code duplication" << std::endl;
    std::cout << "• Code coverage measures test completeness" << std::endl;
    std::cout << "• Good tests are fast, independent, and maintainable" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Unit tests are essential for reliable software development
- Google Test provides powerful and flexible testing framework
- Test organization and naming conventions improve maintainability
- Code coverage helps identify untested code paths
- Test fixtures and parameterized tests reduce duplication
- Testing should be integrated into the build process
- Good tests serve as documentation for expected behavior
- Continuous testing catches regressions early
*/
