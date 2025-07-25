/*
 * C++ NETWORK AND WEB - JSON PARSING WITH NLOHMANN/JSON
 * 
 * This file demonstrates comprehensive JSON processing using the nlohmann/json
 * library, including parsing, serialization, and advanced features.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra json_parsing_nlohmann.cpp -o json_demo
 * 
 * Note: nlohmann/json is header-only, include via: #include <nlohmann/json.hpp>
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

// Mock nlohmann::json interface for demonstration
namespace nlohmann {
    class json {
    private:
        std::map<std::string, std::string> object_data_;
        std::vector<std::string> array_data_;
        std::string string_value_;
        double number_value_ = 0.0;
        bool bool_value_ = false;
        bool is_null_ = false;
        
        enum Type { OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NULL_TYPE } type_ = NULL_TYPE;
        
    public:
        // Constructors
        json() : type_(NULL_TYPE), is_null_(true) {}
        json(const std::string& s) : string_value_(s), type_(STRING) {}
        json(const char* s) : string_value_(s), type_(STRING) {}
        json(double d) : number_value_(d), type_(NUMBER) {}
        json(int i) : number_value_(i), type_(NUMBER) {}
        json(bool b) : bool_value_(b), type_(BOOLEAN) {}
        
        // Object access
        json& operator[](const std::string& key) {
            if (type_ != OBJECT) {
                type_ = OBJECT;
                object_data_.clear();
            }
            return *this; // Simplified for demo
        }
        
        const json& operator[](const std::string& key) const {
            static json null_json;
            return null_json;
        }
        
        // Array access
        json& operator[](size_t index) {
            if (type_ != ARRAY) {
                type_ = ARRAY;
                array_data_.clear();
            }
            return *this; // Simplified for demo
        }
        
        // Type checking
        bool is_null() const { return is_null_; }
        bool is_object() const { return type_ == OBJECT; }
        bool is_array() const { return type_ == ARRAY; }
        bool is_string() const { return type_ == STRING; }
        bool is_number() const { return type_ == NUMBER; }
        bool is_boolean() const { return type_ == BOOLEAN; }
        
        // Value extraction
        std::string get<std::string>() const { return string_value_; }
        double get<double>() const { return number_value_; }
        int get<int>() const { return static_cast<int>(number_value_); }
        bool get<bool>() const { return bool_value_; }
        
        // Serialization
        std::string dump(int indent = -1) const {
            if (indent >= 0) {
                return "{\n  \"formatted\": \"json\"\n}";
            }
            return "{\"compact\":\"json\"}";
        }
        
        // Parsing
        static json parse(const std::string& str) {
            json result;
            // Simplified parsing for demo
            if (str.find('{') != std::string::npos) {
                result.type_ = OBJECT;
            }
            return result;
        }
        
        // Container operations
        void push_back(const json& value) {
            if (type_ != ARRAY) {
                type_ = ARRAY;
                array_data_.clear();
            }
            array_data_.push_back(value.dump());
        }
        
        size_t size() const {
            switch (type_) {
                case OBJECT: return object_data_.size();
                case ARRAY: return array_data_.size();
                default: return 0;
            }
        }
        
        bool empty() const { return size() == 0; }
        
        // Iterator support (simplified)
        auto begin() { return object_data_.begin(); }
        auto end() { return object_data_.end(); }
        auto begin() const { return object_data_.begin(); }
        auto end() const { return object_data_.end(); }
    };
    
    // Template specializations for get<T>()
    template<> std::string json::get<std::string>() const { return string_value_; }
    template<> double json::get<double>() const { return number_value_; }
    template<> int json::get<int>() const { return static_cast<int>(number_value_); }
    template<> bool json::get<bool>() const { return bool_value_; }
}

using json = nlohmann::json;

// =============================================================================
// BASIC JSON OPERATIONS
// =============================================================================

void demonstrate_basic_operations() {
    std::cout << "\n=== Basic JSON Operations ===" << std::endl;
    
    // Creating JSON objects
    std::cout << "\n1. Creating JSON Objects:" << std::endl;
    
    json person;
    person["name"] = "John Doe";
    person["age"] = 30;
    person["email"] = "john@example.com";
    person["active"] = true;
    
    std::cout << "Person JSON: " << person.dump(2) << std::endl;
    
    // Creating JSON arrays
    std::cout << "\n2. Creating JSON Arrays:" << std::endl;
    
    json skills = json::array();
    skills.push_back("C++");
    skills.push_back("Python");
    skills.push_back("JavaScript");
    
    std::cout << "Skills array: " << skills.dump() << std::endl;
    
    // Nested JSON
    std::cout << "\n3. Nested JSON Objects:" << std::endl;
    
    json employee;
    employee["personal"]["name"] = "Jane Smith";
    employee["personal"]["age"] = 28;
    employee["work"]["department"] = "Engineering";
    employee["work"]["position"] = "Senior Developer";
    employee["skills"] = skills;
    
    std::cout << "Employee JSON: " << employee.dump(2) << std::endl;
    
    // Alternative construction methods
    std::cout << "\n4. Alternative Construction:" << std::endl;
    
    // Using initializer lists
    json config = {
        {"server", {
            {"host", "localhost"},
            {"port", 8080},
            {"ssl", false}
        }},
        {"database", {
            {"type", "postgresql"},
            {"host", "db.example.com"},
            {"port", 5432}
        }}
    };
    
    std::cout << "Config JSON: " << config.dump(2) << std::endl;
}

void demonstrate_parsing() {
    std::cout << "\n=== JSON Parsing ===" << std::endl;
    
    // Parse from string
    std::cout << "\n1. Parsing from String:" << std::endl;
    
    std::string json_string = R"({
        "name": "Alice Johnson",
        "age": 25,
        "languages": ["English", "Spanish", "French"],
        "address": {
            "street": "123 Main St",
            "city": "New York",
            "zipcode": "10001"
        },
        "employed": true,
        "salary": 75000.50
    })";
    
    try {
        json parsed = json::parse(json_string);
        std::cout << "✓ Successfully parsed JSON" << std::endl;
        std::cout << "Name: " << parsed["name"].get<std::string>() << std::endl;
        std::cout << "Age: " << parsed["age"].get<int>() << std::endl;
        std::cout << "City: " << parsed["address"]["city"].get<std::string>() << std::endl;
        
        // Iterate through array
        std::cout << "Languages: ";
        for (const auto& lang : parsed["languages"]) {
            std::cout << lang.get<std::string>() << " ";
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Parsing error: " << e.what() << std::endl;
    }
    
    // Parse from file
    std::cout << "\n2. Parsing from File:" << std::endl;
    
    // Create a sample JSON file
    std::ofstream file("sample.json");
    file << R"({
        "application": "MyApp",
        "version": "1.0.0",
        "settings": {
            "debug": true,
            "max_connections": 100,
            "timeout": 30.5
        }
    })";
    file.close();
    
    try {
        std::ifstream input_file("sample.json");
        json file_json;
        input_file >> file_json;
        
        std::cout << "✓ Successfully loaded from file" << std::endl;
        std::cout << "Application: " << file_json["application"].get<std::string>() << std::endl;
        std::cout << "Debug mode: " << (file_json["settings"]["debug"].get<bool>() ? "ON" : "OFF") << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ File parsing error: " << e.what() << std::endl;
    }
}

// =============================================================================
// ADVANCED JSON FEATURES
// =============================================================================

void demonstrate_type_checking() {
    std::cout << "\n=== Type Checking and Validation ===" << std::endl;
    
    json data = {
        {"string_field", "hello"},
        {"number_field", 42},
        {"float_field", 3.14},
        {"bool_field", true},
        {"null_field", nullptr},
        {"array_field", {1, 2, 3}},
        {"object_field", {{"key", "value"}}}
    };
    
    std::cout << "\nType checking results:" << std::endl;
    
    for (auto& [key, value] : data.items()) {
        std::cout << key << ": ";
        
        if (value.is_null()) {
            std::cout << "null";
        } else if (value.is_boolean()) {
            std::cout << "boolean (" << value.get<bool>() << ")";
        } else if (value.is_number()) {
            std::cout << "number (" << value.get<double>() << ")";
        } else if (value.is_string()) {
            std::cout << "string (\"" << value.get<std::string>() << "\")";
        } else if (value.is_array()) {
            std::cout << "array (size: " << value.size() << ")";
        } else if (value.is_object()) {
            std::cout << "object (size: " << value.size() << ")";
        }
        
        std::cout << std::endl;
    }
}

void demonstrate_serialization() {
    std::cout << "\n=== Custom Serialization ===" << std::endl;
    
    // Custom class serialization
    struct Person {
        std::string name;
        int age;
        std::vector<std::string> hobbies;
        
        // Convert to JSON
        json to_json() const {
            json j;
            j["name"] = name;
            j["age"] = age;
            j["hobbies"] = hobbies;
            return j;
        }
        
        // Create from JSON
        static Person from_json(const json& j) {
            Person p;
            p.name = j["name"].get<std::string>();
            p.age = j["age"].get<int>();
            
            for (const auto& hobby : j["hobbies"]) {
                p.hobbies.push_back(hobby.get<std::string>());
            }
            
            return p;
        }
    };
    
    // Serialize object
    Person person{"Bob Wilson", 35, {"reading", "hiking", "programming"}};
    json person_json = person.to_json();
    
    std::cout << "\nSerialized person:" << std::endl;
    std::cout << person_json.dump(2) << std::endl;
    
    // Deserialize object
    Person restored_person = Person::from_json(person_json);
    std::cout << "\nDeserialized person:" << std::endl;
    std::cout << "Name: " << restored_person.name << std::endl;
    std::cout << "Age: " << restored_person.age << std::endl;
    std::cout << "Hobbies: ";
    for (const auto& hobby : restored_person.hobbies) {
        std::cout << hobby << " ";
    }
    std::cout << std::endl;
}

void demonstrate_json_manipulation() {
    std::cout << "\n=== JSON Manipulation ===" << std::endl;
    
    json document = {
        {"users", {
            {{"id", 1}, {"name", "Alice"}, {"active", true}},
            {{"id", 2}, {"name", "Bob"}, {"active", false}},
            {{"id", 3}, {"name", "Charlie"}, {"active", true}}
        }},
        {"metadata", {
            {"version", "1.0"},
            {"created", "2023-01-01"}
        }}
    };
    
    std::cout << "\n1. Original document:" << std::endl;
    std::cout << document.dump(2) << std::endl;
    
    // Add new user
    std::cout << "\n2. Adding new user:" << std::endl;
    json new_user = {{"id", 4}, {"name", "Diana"}, {"active", true}};
    document["users"].push_back(new_user);
    
    // Update metadata
    document["metadata"]["modified"] = "2023-12-01";
    document["metadata"]["version"] = "1.1";
    
    // Remove inactive users
    std::cout << "\n3. Filtering active users:" << std::endl;
    json active_users = json::array();
    for (const auto& user : document["users"]) {
        if (user["active"].get<bool>()) {
            active_users.push_back(user);
        }
    }
    document["users"] = active_users;
    
    std::cout << "Updated document:" << std::endl;
    std::cout << document.dump(2) << std::endl;
}

// =============================================================================
// JSON SCHEMA VALIDATION (CONCEPT)
// =============================================================================

class JsonValidator {
private:
    json schema_;
    
public:
    JsonValidator(const json& schema) : schema_(schema) {}
    
    struct ValidationResult {
        bool valid = true;
        std::vector<std::string> errors;
    };
    
    ValidationResult validate(const json& data) const {
        ValidationResult result;
        
        // Simplified validation logic
        if (schema_.contains("required")) {
            for (const auto& field : schema_["required"]) {
                std::string field_name = field.get<std::string>();
                if (!data.contains(field_name)) {
                    result.valid = false;
                    result.errors.push_back("Missing required field: " + field_name);
                }
            }
        }
        
        if (schema_.contains("properties")) {
            for (const auto& [prop_name, prop_schema] : schema_["properties"].items()) {
                if (data.contains(prop_name)) {
                    // Type validation
                    if (prop_schema.contains("type")) {
                        std::string expected_type = prop_schema["type"].get<std::string>();
                        if (!validate_type(data[prop_name], expected_type)) {
                            result.valid = false;
                            result.errors.push_back("Invalid type for field: " + prop_name);
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
private:
    bool validate_type(const json& value, const std::string& expected_type) const {
        if (expected_type == "string") return value.is_string();
        if (expected_type == "number") return value.is_number();
        if (expected_type == "boolean") return value.is_boolean();
        if (expected_type == "array") return value.is_array();
        if (expected_type == "object") return value.is_object();
        if (expected_type == "null") return value.is_null();
        return false;
    }
};

void demonstrate_validation() {
    std::cout << "\n=== JSON Schema Validation ===" << std::endl;
    
    // Define schema
    json user_schema = {
        {"type", "object"},
        {"required", {"name", "email", "age"}},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"email", {{"type", "string"}}},
            {"age", {{"type", "number"}}},
            {"active", {{"type", "boolean"}}}
        }}
    };
    
    JsonValidator validator(user_schema);
    
    // Valid data
    std::cout << "\n1. Validating valid data:" << std::endl;
    json valid_user = {
        {"name", "John Doe"},
        {"email", "john@example.com"},
        {"age", 30},
        {"active", true}
    };
    
    auto result = validator.validate(valid_user);
    if (result.valid) {
        std::cout << "✓ Validation passed" << std::endl;
    } else {
        std::cout << "✗ Validation failed:" << std::endl;
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << std::endl;
        }
    }
    
    // Invalid data
    std::cout << "\n2. Validating invalid data:" << std::endl;
    json invalid_user = {
        {"name", "Jane Doe"},
        {"age", "thirty"}  // Wrong type
        // Missing required email field
    };
    
    result = validator.validate(invalid_user);
    if (result.valid) {
        std::cout << "✓ Validation passed" << std::endl;
    } else {
        std::cout << "✗ Validation failed:" << std::endl;
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << std::endl;
        }
    }
}

// =============================================================================
// PERFORMANCE AND BEST PRACTICES
// =============================================================================

void demonstrate_performance_tips() {
    std::cout << "\n=== Performance Tips ===" << std::endl;
    
    std::cout << "\n1. Parsing Performance:" << std::endl;
    std::cout << "• Use json::parse() for strings" << std::endl;
    std::cout << "• Use stream operators for files" << std::endl;
    std::cout << "• Consider json::accept() for validation-only" << std::endl;
    std::cout << "• Use json::sax_parse() for streaming large files" << std::endl;
    
    std::cout << "\n2. Memory Usage:" << std::endl;
    std::cout << "• Use json::value_t to check types before conversion" << std::endl;
    std::cout << "• Consider std::move() for large objects" << std::endl;
    std::cout << "• Use references to avoid copying" << std::endl;
    std::cout << "• Clear unused JSON objects explicitly" << std::endl;
    
    std::cout << "\n3. Serialization Options:" << std::endl;
    std::cout << "• dump() without indent for compact output" << std::endl;
    std::cout << "• dump(indent) for pretty-printing" << std::endl;
    std::cout << "• Use stream operators for direct file output" << std::endl;
    
    std::cout << "\n4. Error Handling Best Practices:" << std::endl;
    std::cout << R"(
try {
    json data = json::parse(input);
    // Process data
} catch (const json::parse_error& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
} catch (const json::type_error& e) {
    std::cerr << "Type error: " << e.what() << std::endl;
} catch (const json::out_of_range& e) {
    std::cerr << "Out of range: " << e.what() << std::endl;
}
)" << std::endl;
}

void demonstrate_cmake_integration() {
    std::cout << "\n=== CMake Integration ===" << std::endl;
    
    std::cout << R"(
# Method 1: FetchContent (CMake 3.14+)
include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2)

FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Method 2: find_package (if installed)
find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Method 3: Header-only inclusion
target_include_directories(${PROJECT_NAME} PRIVATE path/to/nlohmann/json/include)
)" << std::endl;
}

int main() {
    std::cout << "JSON PARSING WITH NLOHMANN/JSON\n";
    std::cout << "================================\n";
    
    demonstrate_basic_operations();
    demonstrate_parsing();
    demonstrate_type_checking();
    demonstrate_serialization();
    demonstrate_json_manipulation();
    demonstrate_validation();
    demonstrate_performance_tips();
    demonstrate_cmake_integration();
    
    std::cout << "\nKey JSON Processing Concepts:" << std::endl;
    std::cout << "• nlohmann/json provides intuitive C++ JSON interface" << std::endl;
    std::cout << "• Type safety and validation prevent runtime errors" << std::endl;
    std::cout << "• Custom serialization enables clean object mapping" << std::endl;
    std::cout << "• Performance considerations matter for large datasets" << std::endl;
    std::cout << "• Proper error handling ensures robust applications" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- nlohmann/json is the most popular C++ JSON library
- Header-only design simplifies integration
- Intuitive API similar to JavaScript JSON handling
- Strong type safety with runtime type checking
- Excellent performance for most use cases
- Comprehensive error handling with specific exception types
- Easy integration with modern CMake projects
- Supports both SAX and DOM parsing approaches
*/
