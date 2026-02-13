#pragma once

#include <entt/entity/registry.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "scene/entity.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    class CameraController;

    class Scene
    {
    public:
        Scene();
        ~Scene() noexcept = default;

        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = default;
        Scene& operator=(Scene&&) noexcept = default;

        void tick(float delta_time);

        // Entity management
        Entity createEntity(const std::string& name);
        void   destroyEntity(entt::entity entity);
        Entity findEntity(const std::string& name) const;
        bool   valid(entt::entity entity) const;

        // Wrap a raw entt::entity handle into an Entity helper
        Entity entity(entt::entity handle) { return Entity(handle, &m_registry); }

        // Component access (thin wrappers around entt::registry)
        template<typename T, typename... Args>
        T& emplace(entt::entity entity, Args&&... args);

        template<typename T, typename... Args>
        T& emplaceOrReplace(entt::entity entity, Args&&... args);

        template<typename T>
        T& get(entt::entity entity);
        template<typename T>
        const T& get(entt::entity entity) const;

        template<typename T>
        T* tryGet(entt::entity entity);
        template<typename T>
        const T* tryGet(entt::entity entity) const;

        template<typename T>
        bool has(entt::entity entity) const;

        template<typename T>
        void remove(entt::entity entity);

        // Registry access for views/groups
        entt::registry&       getRegistry() { return m_registry; }
        const entt::registry& getRegistry() const { return m_registry; }

        // Scene hierarchy
        std::shared_ptr<SceneNode> getRoot() const { return m_root; }

        std::shared_ptr<SceneNode> createNode(const std::string& name);
        std::shared_ptr<SceneNode> createNodeWithEntity(const std::string& name);

        std::shared_ptr<CameraController> getCameraController() const { return m_camera_controller; }

    private:
        entt::registry                                m_registry;
        std::unordered_map<std::string, entt::entity> m_name_index;
        std::shared_ptr<SceneNode>                    m_root;
        std::shared_ptr<CameraController>             m_camera_controller;
    };

    // Template implementations

    template<typename T, typename... Args>
    T& Scene::emplace(entt::entity entity, Args&&... args)
    {
        return m_registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T& Scene::emplaceOrReplace(entt::entity entity, Args&&... args)
    {
        return m_registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    T& Scene::get(entt::entity entity)
    {
        return m_registry.get<T>(entity);
    }

    template<typename T>
    const T& Scene::get(entt::entity entity) const
    {
        return m_registry.get<T>(entity);
    }

    template<typename T>
    T* Scene::tryGet(entt::entity entity)
    {
        return m_registry.try_get<T>(entity);
    }

    template<typename T>
    const T* Scene::tryGet(entt::entity entity) const
    {
        return m_registry.try_get<T>(entity);
    }

    template<typename T>
    bool Scene::has(entt::entity entity) const
    {
        return m_registry.all_of<T>(entity);
    }

    template<typename T>
    void Scene::remove(entt::entity entity)
    {
        m_registry.remove<T>(entity);
    }

} // namespace RealmEngine
