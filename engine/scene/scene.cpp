#include "scene/scene.h"

#include "scene/components/camera_controller.h"
#include "scene/components/name_tag.h"
#include "scene/systems/hierarchy_system.h"
#include "scene/systems/transform_system.h"

namespace RealmEngine
{
    Scene::Scene()
    {
        m_root              = std::make_shared<SceneNode>("Root");
        m_camera_controller = std::make_shared<CameraController>();
    }

    void Scene::tick(float delta_time)
    {
        // Run ECS systems each frame
        HierarchySystem::update(*this);
        TransformSystem::update(*this);

        if (m_camera_controller)
            m_camera_controller->update(delta_time);
    }

    Entity Scene::createEntity(const std::string& name)
    {
        // Return existing entity if name already taken
        auto it = m_name_index.find(name);
        if (it != m_name_index.end())
            return Entity(it->second, &m_registry);

        auto handle = m_registry.create();
        m_registry.emplace<NameTag>(handle, NameTag {name});
        m_name_index[name] = handle;
        return Entity(handle, &m_registry);
    }

    void Scene::destroyEntity(entt::entity entity)
    {
        // Remove from name index
        auto* tag = m_registry.try_get<NameTag>(entity);
        if (tag)
            m_name_index.erase(tag->name);

        m_registry.destroy(entity);
    }

    Entity Scene::findEntity(const std::string& name) const
    {
        auto it = m_name_index.find(name);
        if (it != m_name_index.end())
            return Entity(it->second, const_cast<entt::registry*>(&m_registry));
        return Entity();
    }

    bool Scene::valid(entt::entity entity) const { return m_registry.valid(entity); }

    std::shared_ptr<SceneNode> Scene::createNode(const std::string& name) { return std::make_shared<SceneNode>(name); }

    std::shared_ptr<SceneNode> Scene::createNodeWithEntity(const std::string& name)
    {
        auto entity = createEntity(name);
        auto node   = std::make_shared<SceneNode>(name);
        node->setEntity(entity.handle());
        return node;
    }

} // namespace RealmEngine
