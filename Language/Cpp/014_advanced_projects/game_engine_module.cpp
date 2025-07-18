/*
 * ADVANCED PROJECTS - GAME ENGINE MODULE
 * 
 * Modern game engine architecture with Entity-Component-System (ECS),
 * resource management, and performance optimization.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread game_engine_module.cpp -o game_engine
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <bitset>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

// =============================================================================
// MATH UTILITIES
// =============================================================================

struct Vector3 {
    float x, y, z;
    
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    Vector3 normalized() const {
        float mag = magnitude();
        return mag > 0 ? Vector3(x / mag, y / mag, z / mag) : Vector3();
    }
};

struct Transform {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale{1, 1, 1};
    
    Transform(const Vector3& pos = Vector3()) : position(pos) {}
};

// =============================================================================
// ENTITY-COMPONENT-SYSTEM ARCHITECTURE
// =============================================================================

using EntityID = uint32_t;
using ComponentType = std::type_index;

const EntityID INVALID_ENTITY = 0;
const size_t MAX_COMPONENTS = 32;

using ComponentMask = std::bitset<MAX_COMPONENTS>;

class Component {
public:
    virtual ~Component() = default;
    static std::atomic<size_t> next_id_;
    
    template<typename T>
    static size_t get_component_id() {
        static size_t id = next_id_++;
        return id;
    }
};

std::atomic<size_t> Component::next_id_{0};

// Component types
struct TransformComponent : public Component {
    Transform transform;
    TransformComponent(const Transform& t = Transform()) : transform(t) {}
};

struct RenderComponent : public Component {
    std::string mesh_name;
    std::string texture_name;
    bool visible = true;
    
    RenderComponent(const std::string& mesh = "", const std::string& texture = "")
        : mesh_name(mesh), texture_name(texture) {}
};

struct PhysicsComponent : public Component {
    Vector3 velocity;
    Vector3 acceleration;
    float mass = 1.0f;
    bool is_static = false;
    
    PhysicsComponent(float m = 1.0f) : mass(m) {}
};

struct HealthComponent : public Component {
    int current_health = 100;
    int max_health = 100;
    
    HealthComponent(int health = 100) : current_health(health), max_health(health) {}
};

// =============================================================================
// ENTITY MANAGER
// =============================================================================

class EntityManager {
private:
    std::queue<EntityID> available_entities_;
    std::vector<ComponentMask> entity_masks_;
    EntityID next_entity_id_ = 1;
    size_t living_entities_ = 0;
    
public:
    EntityManager() {
        entity_masks_.resize(1000); // Pre-allocate for performance
    }
    
    EntityID create_entity() {
        EntityID entity_id;
        
        if (!available_entities_.empty()) {
            entity_id = available_entities_.front();
            available_entities_.pop();
        } else {
            entity_id = next_entity_id_++;
            if (entity_id >= entity_masks_.size()) {
                entity_masks_.resize(entity_masks_.size() * 2);
            }
        }
        
        entity_masks_[entity_id].reset();
        ++living_entities_;
        
        std::cout << "Created entity " << entity_id << std::endl;
        return entity_id;
    }
    
    void destroy_entity(EntityID entity) {
        if (entity < entity_masks_.size()) {
            entity_masks_[entity].reset();
            available_entities_.push(entity);
            --living_entities_;
            std::cout << "Destroyed entity " << entity << std::endl;
        }
    }
    
    void set_component_mask(EntityID entity, const ComponentMask& mask) {
        if (entity < entity_masks_.size()) {
            entity_masks_[entity] = mask;
        }
    }
    
    ComponentMask get_component_mask(EntityID entity) const {
        return entity < entity_masks_.size() ? entity_masks_[entity] : ComponentMask();
    }
    
    size_t get_living_entity_count() const { return living_entities_; }
};

// =============================================================================
// COMPONENT MANAGER
// =============================================================================

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void entity_destroyed(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
private:
    std::vector<T> components_;
    std::unordered_map<EntityID, size_t> entity_to_index_;
    std::unordered_map<size_t, EntityID> index_to_entity_;
    size_t size_ = 0;
    
public:
    void insert(EntityID entity, T component) {
        if (entity_to_index_.find(entity) != entity_to_index_.end()) {
            return; // Component already exists
        }
        
        size_t new_index = size_;
        entity_to_index_[entity] = new_index;
        index_to_entity_[new_index] = entity;
        
        if (new_index >= components_.size()) {
            components_.resize(components_.size() * 2 + 1);
        }
        
        components_[new_index] = component;
        ++size_;
    }
    
    void remove(EntityID entity) {
        auto it = entity_to_index_.find(entity);
        if (it == entity_to_index_.end()) {
            return; // Component doesn't exist
        }
        
        size_t index_to_remove = it->second;
        size_t last_index = size_ - 1;
        
        // Move last element to removed position
        components_[index_to_remove] = components_[last_index];
        
        // Update mappings
        EntityID last_entity = index_to_entity_[last_index];
        entity_to_index_[last_entity] = index_to_remove;
        index_to_entity_[index_to_remove] = last_entity;
        
        // Clean up
        entity_to_index_.erase(entity);
        index_to_entity_.erase(last_index);
        --size_;
    }
    
    T& get(EntityID entity) {
        auto it = entity_to_index_.find(entity);
        if (it == entity_to_index_.end()) {
            throw std::runtime_error("Component not found for entity");
        }
        return components_[it->second];
    }
    
    void entity_destroyed(EntityID entity) override {
        remove(entity);
    }
    
    std::vector<T>& get_components() { return components_; }
    const std::vector<EntityID> get_entities() const {
        std::vector<EntityID> entities;
        for (const auto& pair : entity_to_index_) {
            entities.push_back(pair.first);
        }
        return entities;
    }
};

class ComponentManager {
private:
    std::unordered_map<ComponentType, std::unique_ptr<IComponentArray>> component_arrays_;
    std::unordered_map<ComponentType, size_t> component_ids_;
    size_t next_component_id_ = 0;
    
public:
    template<typename T>
    void register_component() {
        ComponentType type = std::type_index(typeid(T));
        
        if (component_arrays_.find(type) != component_arrays_.end()) {
            return; // Already registered
        }
        
        component_arrays_[type] = std::make_unique<ComponentArray<T>>();
        component_ids_[type] = next_component_id_++;
        
        std::cout << "Registered component: " << type.name() << std::endl;
    }
    
    template<typename T>
    void add_component(EntityID entity, T component) {
        get_component_array<T>()->insert(entity, component);
    }
    
    template<typename T>
    void remove_component(EntityID entity) {
        get_component_array<T>()->remove(entity);
    }
    
    template<typename T>
    T& get_component(EntityID entity) {
        return get_component_array<T>()->get(entity);
    }
    
    template<typename T>
    size_t get_component_id() {
        ComponentType type = std::type_index(typeid(T));
        return component_ids_[type];
    }
    
    void entity_destroyed(EntityID entity) {
        for (auto& pair : component_arrays_) {
            pair.second->entity_destroyed(entity);
        }
    }
    
private:
    template<typename T>
    ComponentArray<T>* get_component_array() {
        ComponentType type = std::type_index(typeid(T));
        auto it = component_arrays_.find(type);
        if (it == component_arrays_.end()) {
            throw std::runtime_error("Component not registered");
        }
        return static_cast<ComponentArray<T>*>(it->second.get());
    }
};

// =============================================================================
// SYSTEM BASE CLASS AND IMPLEMENTATIONS
// =============================================================================

class System {
public:
    std::vector<EntityID> entities_;
    
    virtual ~System() = default;
    virtual void update(float delta_time) = 0;
    virtual void init() {}
    virtual void shutdown() {}
};

class RenderSystem : public System {
public:
    void update(float delta_time) override {
        // Mock rendering
        static int frame_count = 0;
        if (++frame_count % 60 == 0) { // Print every 60 frames
            std::cout << "Rendering " << entities_.size() << " entities" << std::endl;
        }
    }
    
    void render_entity(EntityID entity, const TransformComponent& transform, 
                      const RenderComponent& render) {
        if (!render.visible) return;
        
        // Mock rendering logic
        std::cout << "Rendering entity " << entity 
                  << " at (" << transform.transform.position.x 
                  << ", " << transform.transform.position.y 
                  << ", " << transform.transform.position.z << ")" << std::endl;
    }
};

class PhysicsSystem : public System {
public:
    void update(float delta_time) override {
        // Mock physics simulation
        for (EntityID entity : entities_) {
            // In real implementation, would update physics for each entity
            simulate_physics(entity, delta_time);
        }
    }
    
private:
    void simulate_physics(EntityID entity, float delta_time) {
        // Mock physics calculations
        static int physics_steps = 0;
        if (++physics_steps % 120 == 0) { // Print every 120 steps
            std::cout << "Physics simulation for entity " << entity 
                      << " (dt: " << delta_time << ")" << std::endl;
        }
    }
};

class HealthSystem : public System {
public:
    void update(float delta_time) override {
        // Mock health regeneration
        for (EntityID entity : entities_) {
            regenerate_health(entity, delta_time);
        }
    }
    
private:
    void regenerate_health(EntityID entity, float delta_time) {
        // Mock health regeneration logic
        static float regen_timer = 0;
        regen_timer += delta_time;
        
        if (regen_timer >= 1.0f) { // Regenerate every second
            std::cout << "Health regeneration for entity " << entity << std::endl;
            regen_timer = 0;
        }
    }
};

// =============================================================================
// SYSTEM MANAGER
// =============================================================================

class SystemManager {
private:
    std::unordered_map<std::type_index, std::unique_ptr<System>> systems_;
    std::unordered_map<std::type_index, ComponentMask> system_masks_;
    
public:
    template<typename T>
    void register_system() {
        std::type_index type = std::type_index(typeid(T));
        
        if (systems_.find(type) != systems_.end()) {
            return; // Already registered
        }
        
        systems_[type] = std::make_unique<T>();
        systems_[type]->init();
        
        std::cout << "Registered system: " << type.name() << std::endl;
    }
    
    template<typename T>
    void set_system_mask(ComponentMask mask) {
        std::type_index type = std::type_index(typeid(T));
        system_masks_[type] = mask;
    }
    
    void entity_mask_changed(EntityID entity, ComponentMask mask) {
        for (auto& [type, system] : systems_) {
            ComponentMask system_mask = system_masks_[type];
            
            if ((mask & system_mask) == system_mask) {
                // Entity has all required components
                auto& entities = system->entities_;
                if (std::find(entities.begin(), entities.end(), entity) == entities.end()) {
                    entities.push_back(entity);
                }
            } else {
                // Entity doesn't have required components
                auto& entities = system->entities_;
                entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
            }
        }
    }
    
    void update_all_systems(float delta_time) {
        for (auto& [type, system] : systems_) {
            system->update(delta_time);
        }
    }
    
    void shutdown_all_systems() {
        for (auto& [type, system] : systems_) {
            system->shutdown();
        }
    }
};

// =============================================================================
// GAME ENGINE COORDINATOR
// =============================================================================

class GameEngine {
private:
    std::unique_ptr<EntityManager> entity_manager_;
    std::unique_ptr<ComponentManager> component_manager_;
    std::unique_ptr<SystemManager> system_manager_;
    
    std::atomic<bool> running_;
    std::thread game_loop_thread_;
    
    // Performance metrics
    std::chrono::steady_clock::time_point last_frame_time_;
    float delta_time_ = 0.0f;
    int fps_ = 0;
    int frame_count_ = 0;
    
public:
    GameEngine() : running_(false) {
        entity_manager_ = std::make_unique<EntityManager>();
        component_manager_ = std::make_unique<ComponentManager>();
        system_manager_ = std::make_unique<SystemManager>();
        
        initialize_components();
        initialize_systems();
        
        std::cout << "Game Engine initialized" << std::endl;
    }
    
    ~GameEngine() {
        stop();
        std::cout << "Game Engine destroyed" << std::endl;
    }
    
    // Entity operations
    EntityID create_entity() {
        return entity_manager_->create_entity();
    }
    
    void destroy_entity(EntityID entity) {
        entity_manager_->destroy_entity(entity);
        component_manager_->entity_destroyed(entity);
    }
    
    // Component operations
    template<typename T>
    void add_component(EntityID entity, T component) {
        component_manager_->add_component<T>(entity, component);
        
        ComponentMask mask = entity_manager_->get_component_mask(entity);
        mask.set(component_manager_->get_component_id<T>());
        entity_manager_->set_component_mask(entity, mask);
        
        system_manager_->entity_mask_changed(entity, mask);
    }
    
    template<typename T>
    void remove_component(EntityID entity) {
        component_manager_->remove_component<T>(entity);
        
        ComponentMask mask = entity_manager_->get_component_mask(entity);
        mask.reset(component_manager_->get_component_id<T>());
        entity_manager_->set_component_mask(entity, mask);
        
        system_manager_->entity_mask_changed(entity, mask);
    }
    
    template<typename T>
    T& get_component(EntityID entity) {
        return component_manager_->get_component<T>(entity);
    }
    
    // Game loop
    void start() {
        if (running_) return;
        
        std::cout << "Starting game engine..." << std::endl;
        running_ = true;
        last_frame_time_ = std::chrono::steady_clock::now();
        
        game_loop_thread_ = std::thread([this] { game_loop(); });
    }
    
    void stop() {
        if (!running_) return;
        
        std::cout << "Stopping game engine..." << std::endl;
        running_ = false;
        
        if (game_loop_thread_.joinable()) {
            game_loop_thread_.join();
        }
        
        system_manager_->shutdown_all_systems();
    }
    
    void print_stats() {
        std::cout << "\n=== Game Engine Stats ===" << std::endl;
        std::cout << "Entities: " << entity_manager_->get_living_entity_count() << std::endl;
        std::cout << "FPS: " << fps_ << std::endl;
        std::cout << "Delta Time: " << delta_time_ << "s" << std::endl;
        std::cout << "=========================" << std::endl;
    }
    
private:
    void initialize_components() {
        component_manager_->register_component<TransformComponent>();
        component_manager_->register_component<RenderComponent>();
        component_manager_->register_component<PhysicsComponent>();
        component_manager_->register_component<HealthComponent>();
    }
    
    void initialize_systems() {
        system_manager_->register_system<RenderSystem>();
        system_manager_->register_system<PhysicsSystem>();
        system_manager_->register_system<HealthSystem>();
        
        // Set system component requirements
        ComponentMask render_mask;
        render_mask.set(component_manager_->get_component_id<TransformComponent>());
        render_mask.set(component_manager_->get_component_id<RenderComponent>());
        system_manager_->set_system_mask<RenderSystem>(render_mask);
        
        ComponentMask physics_mask;
        physics_mask.set(component_manager_->get_component_id<TransformComponent>());
        physics_mask.set(component_manager_->get_component_id<PhysicsComponent>());
        system_manager_->set_system_mask<PhysicsSystem>(physics_mask);
        
        ComponentMask health_mask;
        health_mask.set(component_manager_->get_component_id<HealthComponent>());
        system_manager_->set_system_mask<HealthSystem>(health_mask);
    }
    
    void game_loop() {
        std::cout << "Game loop started" << std::endl;
        
        while (running_) {
            auto current_time = std::chrono::steady_clock::now();
            delta_time_ = std::chrono::duration<float>(current_time - last_frame_time_).count();
            last_frame_time_ = current_time;
            
            // Update all systems
            system_manager_->update_all_systems(delta_time_);
            
            // Calculate FPS
            ++frame_count_;
            static auto fps_timer = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(current_time - fps_timer).count() >= 1) {
                fps_ = frame_count_;
                frame_count_ = 0;
                fps_timer = current_time;
            }
            
            // Target 60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        std::cout << "Game loop stopped" << std::endl;
    }
};

// =============================================================================
// EXAMPLE GAME OBJECTS
// =============================================================================

void create_sample_entities(GameEngine& engine) {
    std::cout << "\n=== Creating Sample Entities ===" << std::endl;
    
    // Create player entity
    EntityID player = engine.create_entity();
    engine.add_component(player, TransformComponent(Transform(Vector3(0, 0, 0))));
    engine.add_component(player, RenderComponent("player_mesh", "player_texture"));
    engine.add_component(player, PhysicsComponent(1.0f));
    engine.add_component(player, HealthComponent(100));
    std::cout << "Created player entity: " << player << std::endl;
    
    // Create enemy entities
    for (int i = 0; i < 5; ++i) {
        EntityID enemy = engine.create_entity();
        Vector3 pos(i * 2.0f, 0, 5.0f);
        engine.add_component(enemy, TransformComponent(Transform(pos)));
        engine.add_component(enemy, RenderComponent("enemy_mesh", "enemy_texture"));
        engine.add_component(enemy, PhysicsComponent(0.8f));
        engine.add_component(enemy, HealthComponent(50));
        std::cout << "Created enemy entity: " << enemy << std::endl;
    }
    
    // Create static objects
    for (int i = 0; i < 3; ++i) {
        EntityID obstacle = engine.create_entity();
        Vector3 pos(-3.0f + i * 3.0f, 0, 3.0f);
        engine.add_component(obstacle, TransformComponent(Transform(pos)));
        engine.add_component(obstacle, RenderComponent("obstacle_mesh", "obstacle_texture"));
        std::cout << "Created obstacle entity: " << obstacle << std::endl;
    }
}

int main() {
    std::cout << "GAME ENGINE MODULE - ECS ARCHITECTURE\n";
    std::cout << "=====================================\n";
    
    try {
        // Create game engine
        GameEngine engine;
        
        // Create sample entities
        create_sample_entities(engine);
        
        // Start game engine
        engine.start();
        
        // Let it run for a while
        std::cout << "\nGame running... (will stop in 10 seconds)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // Print statistics
        engine.print_stats();
        
        // Stop engine
        engine.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nKey Features:" << std::endl;
    std::cout << "• Entity-Component-System (ECS) architecture" << std::endl;
    std::cout << "• Type-safe component management" << std::endl;
    std::cout << "• Efficient system updates with entity filtering" << std::endl;
    std::cout << "• Multi-threaded game loop" << std::endl;
    std::cout << "• Performance monitoring and statistics" << std::endl;
    std::cout << "• Scalable entity and component storage" << std::endl;
    
    return 0;
}
