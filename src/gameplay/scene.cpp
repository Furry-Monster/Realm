#include "gameplay/scene.h"

#include <functional>
#include <memory>
#include "gameplay/camera_controller.h"
#include "gameplay/entity.h"
#include "render/render_camera.h"

namespace RealmEngine
{
    Scene::Scene() { m_root = std::make_shared<SceneNode>("Root"); }

    void Scene::initialize(std::shared_ptr<RenderCamera> camera)
    {
        m_camera_controller = std::make_shared<CameraController>();
        m_camera_controller->initialize(camera);
    }

    void Scene::tick(float delta_time)
    {
        if (m_camera_controller)
            m_camera_controller->update(delta_time);
    }

    constexpr size_t Scene::hashName(const std::string& name) { return std::hash<std::string> {}(name); }

    std::shared_ptr<Entity> Scene::createEntity(const std::string& name)
    {
        size_t id = hashName(name);
        if (m_entities.find(id) != m_entities.end())
        {
            return m_entities[id];
        }
        auto entity    = std::make_shared<Entity>(id);
        m_entities[id] = entity;
        return entity;
    }

    std::shared_ptr<Entity> Scene::getEntity(const std::string& name) const
    {
        size_t id = hashName(name);
        return getEntity(id);
    }

    std::shared_ptr<Entity> Scene::getEntity(size_t id) const
    {
        auto it = m_entities.find(id);
        return (it != m_entities.end()) ? it->second : nullptr;
    }

    bool Scene::hasEntity(const std::string& name) const
    {
        size_t id = hashName(name);
        return hasEntity(id);
    }

    bool Scene::hasEntity(size_t id) const { return m_entities.find(id) != m_entities.end(); }

    void Scene::removeEntity(const std::string& name)
    {
        size_t id = hashName(name);
        removeEntity(id);
    }

    void Scene::removeEntity(size_t id)
    {
        auto it = m_entities.find(id);
        if (it != m_entities.end())
        {
            m_entities.erase(it);
        }
    }

    std::shared_ptr<SceneNode> Scene::createNode(const std::string& name, size_t entity_id)
    {
        auto node = std::make_shared<SceneNode>(name);
        if (entity_id != 0)
        {
            node->setEntityId(entity_id);
        }
        return node;
    }

    std::shared_ptr<SceneNode> Scene::createNodeWithEntity(const std::string& name)
    {
        auto entity = createEntity(name);
        auto node   = std::make_shared<SceneNode>(name);
        node->setEntityId(entity->getId());
        return node;
    }
} // namespace RealmEngine
