#pragma once

#include <memory>
#include <unordered_map>
#include "scene/components/camera_controller.h"
#include "scene/entity.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    class RenderCamera;

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

        std::shared_ptr<SceneNode> getRoot() const { return m_root; }

        std::shared_ptr<Entity> createEntity(const std::string& name);
        std::shared_ptr<Entity> getEntity(const std::string& name) const;
        std::shared_ptr<Entity> getEntity(size_t id) const;
        bool                    hasEntity(const std::string& name) const;
        bool                    hasEntity(size_t id) const;
        void                    removeEntity(const std::string& name);
        void                    removeEntity(size_t id);

        std::shared_ptr<SceneNode> createNode(const std::string& name, size_t entity_id = 0);
        std::shared_ptr<SceneNode> createNodeWithEntity(const std::string& name);

        std::shared_ptr<CameraController> getCameraController() const { return m_camera_controller; }

        static size_t hashName(const std::string& name);

    private:
        std::shared_ptr<SceneNode>                          m_root;
        std::unordered_map<size_t, std::shared_ptr<Entity>> m_entities;
        std::shared_ptr<CameraController>                   m_camera_controller;
    };
} // namespace RealmEngine
