/*
 * ADVANCED PROJECTS - PLUGIN ARCHITECTURE WITH SHARED LIBRARIES
 * 
 * Dynamic plugin loading system with shared libraries (.so/.dll).
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -ldl plugin_architecture_so.cpp -o plugin_system
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#define PLUGIN_EXT ".dll"
#define EXPORT __declspec(dllexport)
using LibHandle = HMODULE;
#else
#include <dlfcn.h>
#define PLUGIN_EXT ".so"
#define EXPORT
using LibHandle = void*;
#endif

// =============================================================================
// PLUGIN INTERFACE
// =============================================================================

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string get_name() const = 0;
    virtual std::string get_version() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void execute() = 0;
};

// Plugin factory function type
using CreatePluginFunc = IPlugin*(*)();
using DestroyPluginFunc = void(*)(IPlugin*);

// =============================================================================
// PLUGIN MANAGER
// =============================================================================

class PluginManager {
private:
    struct PluginInfo {
        std::string name;
        std::string path;
        LibHandle handle;
        std::unique_ptr<IPlugin> instance;
        CreatePluginFunc create_func;
        DestroyPluginFunc destroy_func;
        bool loaded = false;
        
        PluginInfo() : handle(nullptr), create_func(nullptr), destroy_func(nullptr) {}
    };
    
    std::unordered_map<std::string, PluginInfo> plugins_;
    std::string plugin_directory_;
    
public:
    PluginManager(const std::string& plugin_dir = "./plugins") 
        : plugin_directory_(plugin_dir) {
        std::cout << "Plugin Manager initialized with directory: " << plugin_dir << std::endl;
    }
    
    ~PluginManager() {
        unload_all_plugins();
    }
    
    bool scan_plugins() {
        std::cout << "\nScanning for plugins in: " << plugin_directory_ << std::endl;
        
        if (!std::filesystem::exists(plugin_directory_)) {
            std::cout << "Plugin directory does not exist, creating mock plugins..." << std::endl;
            create_mock_plugins();
            return true;
        }
        
        size_t found_count = 0;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(plugin_directory_)) {
                if (entry.is_regular_file() && 
                    entry.path().extension() == PLUGIN_EXT) {
                    
                    std::string plugin_name = entry.path().stem().string();
                    std::string plugin_path = entry.path().string();
                    
                    PluginInfo info;
                    info.name = plugin_name;
                    info.path = plugin_path;
                    
                    plugins_[plugin_name] = std::move(info);
                    ++found_count;
                    
                    std::cout << "Found plugin: " << plugin_name << " at " << plugin_path << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error scanning plugins: " << e.what() << std::endl;
            return false;
        }
        
        std::cout << "Found " << found_count << " plugins" << std::endl;
        return true;
    }
    
    bool load_plugin(const std::string& name) {
        auto it = plugins_.find(name);
        if (it == plugins_.end()) {
            std::cerr << "Plugin not found: " << name << std::endl;
            return false;
        }
        
        PluginInfo& info = it->second;
        if (info.loaded) {
            std::cout << "Plugin already loaded: " << name << std::endl;
            return true;
        }
        
        std::cout << "Loading plugin: " << name << std::endl;
        
        // Load shared library
        info.handle = load_library(info.path);
        if (!info.handle) {
            std::cerr << "Failed to load library: " << info.path << std::endl;
            return false;
        }
        
        // Get function pointers
        info.create_func = reinterpret_cast<CreatePluginFunc>(
            get_symbol(info.handle, "create_plugin"));
        info.destroy_func = reinterpret_cast<DestroyPluginFunc>(
            get_symbol(info.handle, "destroy_plugin"));
        
        if (!info.create_func || !info.destroy_func) {
            std::cerr << "Failed to find plugin functions in: " << name << std::endl;
            unload_library(info.handle);
            info.handle = nullptr;
            return false;
        }
        
        // Create plugin instance
        IPlugin* plugin = info.create_func();
        if (!plugin) {
            std::cerr << "Failed to create plugin instance: " << name << std::endl;
            unload_library(info.handle);
            info.handle = nullptr;
            return false;
        }
        
        info.instance.reset(plugin);
        
        // Initialize plugin
        if (!info.instance->initialize()) {
            std::cerr << "Failed to initialize plugin: " << name << std::endl;
            info.destroy_func(info.instance.release());
            unload_library(info.handle);
            info.handle = nullptr;
            return false;
        }
        
        info.loaded = true;
        std::cout << "Successfully loaded plugin: " << name 
                  << " (version: " << info.instance->get_version() << ")" << std::endl;
        
        return true;
    }
    
    bool unload_plugin(const std::string& name) {
        auto it = plugins_.find(name);
        if (it == plugins_.end() || !it->second.loaded) {
            return false;
        }
        
        PluginInfo& info = it->second;
        std::cout << "Unloading plugin: " << name << std::endl;
        
        // Shutdown plugin
        if (info.instance) {
            info.instance->shutdown();
            info.destroy_func(info.instance.release());
        }
        
        // Unload library
        if (info.handle) {
            unload_library(info.handle);
            info.handle = nullptr;
        }
        
        info.loaded = false;
        info.create_func = nullptr;
        info.destroy_func = nullptr;
        
        std::cout << "Plugin unloaded: " << name << std::endl;
        return true;
    }
    
    void unload_all_plugins() {
        std::cout << "\nUnloading all plugins..." << std::endl;
        for (auto& [name, info] : plugins_) {
            if (info.loaded) {
                unload_plugin(name);
            }
        }
    }
    
    bool execute_plugin(const std::string& name) {
        auto it = plugins_.find(name);
        if (it == plugins_.end() || !it->second.loaded) {
            std::cerr << "Plugin not loaded: " << name << std::endl;
            return false;
        }
        
        std::cout << "Executing plugin: " << name << std::endl;
        it->second.instance->execute();
        return true;
    }
    
    std::vector<std::string> get_loaded_plugins() const {
        std::vector<std::string> loaded;
        for (const auto& [name, info] : plugins_) {
            if (info.loaded) {
                loaded.push_back(name);
            }
        }
        return loaded;
    }
    
    std::vector<std::string> get_available_plugins() const {
        std::vector<std::string> available;
        for (const auto& [name, info] : plugins_) {
            available.push_back(name);
        }
        return available;
    }
    
    void list_plugins() const {
        std::cout << "\n=== Plugin Status ===" << std::endl;
        for (const auto& [name, info] : plugins_) {
            std::cout << "Plugin: " << name << std::endl;
            std::cout << "  Path: " << info.path << std::endl;
            std::cout << "  Status: " << (info.loaded ? "LOADED" : "NOT LOADED") << std::endl;
            if (info.loaded && info.instance) {
                std::cout << "  Version: " << info.instance->get_version() << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
private:
    LibHandle load_library(const std::string& path) {
#ifdef _WIN32
        return LoadLibraryA(path.c_str());
#else
        return dlopen(path.c_str(), RTLD_LAZY);
#endif
    }
    
    void* get_symbol(LibHandle handle, const std::string& symbol) {
#ifdef _WIN32
        return GetProcAddress(handle, symbol.c_str());
#else
        return dlsym(handle, symbol.c_str());
#endif
    }
    
    void unload_library(LibHandle handle) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
    
    void create_mock_plugins() {
        // Create mock plugin entries for demonstration
        std::vector<std::string> mock_plugins = {
            "audio_plugin", "graphics_plugin", "network_plugin", "ai_plugin"
        };
        
        for (const auto& name : mock_plugins) {
            PluginInfo info;
            info.name = name;
            info.path = plugin_directory_ + "/" + name + PLUGIN_EXT;
            plugins_[name] = std::move(info);
        }
        
        std::cout << "Created " << mock_plugins.size() << " mock plugin entries" << std::endl;
    }
};

// =============================================================================
// EXAMPLE PLUGIN IMPLEMENTATIONS
// =============================================================================

class AudioPlugin : public IPlugin {
public:
    std::string get_name() const override { return "Audio Plugin"; }
    std::string get_version() const override { return "1.0.0"; }
    
    bool initialize() override {
        std::cout << "Audio Plugin: Initializing audio system..." << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Audio Plugin: Shutting down audio system..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Audio Plugin: Playing background music..." << std::endl;
        std::cout << "♪ ♫ ♪ ♫ Audio is working! ♪ ♫ ♪ ♫" << std::endl;
    }
};

class GraphicsPlugin : public IPlugin {
public:
    std::string get_name() const override { return "Graphics Plugin"; }
    std::string get_version() const override { return "2.1.0"; }
    
    bool initialize() override {
        std::cout << "Graphics Plugin: Initializing OpenGL context..." << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Graphics Plugin: Cleaning up graphics resources..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Graphics Plugin: Rendering 3D scene..." << std::endl;
        std::cout << "█▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀█" << std::endl;
        std::cout << "█  Graphics Rendered!  █" << std::endl;
        std::cout << "█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█" << std::endl;
    }
};

class NetworkPlugin : public IPlugin {
public:
    std::string get_name() const override { return "Network Plugin"; }
    std::string get_version() const override { return "1.5.2"; }
    
    bool initialize() override {
        std::cout << "Network Plugin: Initializing network stack..." << std::endl;
        return true;
    }
    
    void shutdown() override {
        std::cout << "Network Plugin: Closing network connections..." << std::endl;
    }
    
    void execute() override {
        std::cout << "Network Plugin: Sending/receiving data..." << std::endl;
        std::cout << "📡 Network communication active!" << std::endl;
    }
};

// =============================================================================
// PLUGIN REGISTRY (FOR STATIC PLUGINS)
// =============================================================================

class PluginRegistry {
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<IPlugin>()>> factories_;
    
public:
    static PluginRegistry& instance() {
        static PluginRegistry registry;
        return registry;
    }
    
    template<typename T>
    void register_plugin(const std::string& name) {
        factories_[name] = []() { return std::make_unique<T>(); };
        std::cout << "Registered static plugin: " << name << std::endl;
    }
    
    std::unique_ptr<IPlugin> create_plugin(const std::string& name) {
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    std::vector<std::string> get_registered_plugins() const {
        std::vector<std::string> names;
        for (const auto& [name, factory] : factories_) {
            names.push_back(name);
        }
        return names;
    }
};

// =============================================================================
// APPLICATION WITH PLUGIN SUPPORT
// =============================================================================

class Application {
private:
    PluginManager plugin_manager_;
    std::vector<std::unique_ptr<IPlugin>> static_plugins_;
    
public:
    Application() : plugin_manager_("./plugins") {
        std::cout << "Application with plugin support initialized" << std::endl;
        register_static_plugins();
    }
    
    void run() {
        std::cout << "\n=== Application Starting ===" << std::endl;
        
        // Load static plugins
        load_static_plugins();
        
        // Scan and load dynamic plugins
        plugin_manager_.scan_plugins();
        
        // Try to load some plugins
        auto available = plugin_manager_.get_available_plugins();
        for (const auto& plugin_name : available) {
            if (plugin_manager_.load_plugin(plugin_name)) {
                std::cout << "Successfully loaded: " << plugin_name << std::endl;
            }
        }
        
        // List all plugins
        plugin_manager_.list_plugins();
        
        // Execute loaded plugins
        std::cout << "\n=== Executing Plugins ===" << std::endl;
        
        // Execute static plugins
        for (auto& plugin : static_plugins_) {
            plugin->execute();
        }
        
        // Execute dynamic plugins
        auto loaded = plugin_manager_.get_loaded_plugins();
        for (const auto& plugin_name : loaded) {
            plugin_manager_.execute_plugin(plugin_name);
        }
        
        std::cout << "\n=== Application Running ===" << std::endl;
        std::cout << "Application is running with " 
                  << (static_plugins_.size() + loaded.size()) 
                  << " active plugins..." << std::endl;
        
        // Simulate application runtime
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        std::cout << "\n=== Application Shutting Down ===" << std::endl;
        shutdown_static_plugins();
    }
    
private:
    void register_static_plugins() {
        auto& registry = PluginRegistry::instance();
        registry.register_plugin<AudioPlugin>("audio_static");
        registry.register_plugin<GraphicsPlugin>("graphics_static");
        registry.register_plugin<NetworkPlugin>("network_static");
    }
    
    void load_static_plugins() {
        std::cout << "\nLoading static plugins..." << std::endl;
        
        auto& registry = PluginRegistry::instance();
        auto registered = registry.get_registered_plugins();
        
        for (const auto& name : registered) {
            auto plugin = registry.create_plugin(name);
            if (plugin && plugin->initialize()) {
                std::cout << "Loaded static plugin: " << plugin->get_name() 
                          << " v" << plugin->get_version() << std::endl;
                static_plugins_.push_back(std::move(plugin));
            }
        }
    }
    
    void shutdown_static_plugins() {
        std::cout << "Shutting down static plugins..." << std::endl;
        for (auto& plugin : static_plugins_) {
            plugin->shutdown();
        }
        static_plugins_.clear();
    }
};

// =============================================================================
// PLUGIN DEVELOPMENT UTILITIES
// =============================================================================

class PluginDevelopmentTools {
public:
    static void create_plugin_template(const std::string& plugin_name) {
        std::cout << "\n=== Plugin Template Generator ===" << std::endl;
        std::cout << "Generating template for plugin: " << plugin_name << std::endl;
        
        std::string header_content = R"(
#pragma once
#include "plugin_interface.h"

class )" + plugin_name + R"(Plugin : public IPlugin {
public:
    std::string get_name() const override;
    std::string get_version() const override;
    bool initialize() override;
    void shutdown() override;
    void execute() override;
};

// Export functions
extern "C" {
    EXPORT IPlugin* create_plugin();
    EXPORT void destroy_plugin(IPlugin* plugin);
}
)";
        
        std::cout << "Generated header template:" << std::endl;
        std::cout << header_content << std::endl;
    }
    
    static void validate_plugin_interface(IPlugin* plugin) {
        if (!plugin) {
            std::cout << "❌ Plugin validation failed: null pointer" << std::endl;
            return;
        }
        
        std::cout << "\n=== Plugin Interface Validation ===" << std::endl;
        std::cout << "✅ Plugin name: " << plugin->get_name() << std::endl;
        std::cout << "✅ Plugin version: " << plugin->get_version() << std::endl;
        
        if (plugin->initialize()) {
            std::cout << "✅ Plugin initialization: SUCCESS" << std::endl;
            plugin->shutdown();
            std::cout << "✅ Plugin shutdown: SUCCESS" << std::endl;
        } else {
            std::cout << "❌ Plugin initialization: FAILED" << std::endl;
        }
    }
};

int main() {
    std::cout << "PLUGIN ARCHITECTURE WITH SHARED LIBRARIES\n";
    std::cout << "=========================================\n";
    
    try {
        // Create and run application
        Application app;
        app.run();
        
        // Demonstrate plugin development tools
        PluginDevelopmentTools::create_plugin_template("MyCustom");
        
        // Validate plugin interfaces
        auto audio_plugin = std::make_unique<AudioPlugin>();
        PluginDevelopmentTools::validate_plugin_interface(audio_plugin.get());
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nKey Features:" << std::endl;
    std::cout << "• Dynamic plugin loading from shared libraries" << std::endl;
    std::cout << "• Static plugin registration system" << std::endl;
    std::cout << "• Cross-platform library loading (Windows/Linux)" << std::endl;
    std::cout << "• Plugin lifecycle management (load/unload)" << std::endl;
    std::cout << "• Plugin interface validation" << std::endl;
    std::cout << "• Development tools and templates" << std::endl;
    
    return 0;
}
