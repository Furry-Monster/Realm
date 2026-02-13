#include "scene/scene.h"

#include "scene/components/camera_controller.h"
#include "scene/components/name_tag.h"

namespace RealmEngine
{
    Scene::Scene()
    {
        m_root              = std::make_shared<SceneNode>("Root");
        m_camera_controller = std::make_shared<CameraController>();
    }

    void Scene::tick(float delta_time)
    {
        if (m_camera_controller)
            m_camera_controller->update(delta_time);
    }

    entt::entity Scene::createEntity(const std::string& name)
    {
        // Return existing entity if name already taken
        auto it = m_name_index.find(name);
        if (it != m_name_index.end())
            return it->second;

        auto entity = m_registry.create();
        m_registry.emplace<NameTag>(entity, NameTag {name});
        m_name_index[name] = entity;
        return entity;
    }

    void Scene::destroyEntity(entt::entity entity)
    {
        // Remove from name index
        auto* tag = m_registry.try_get<NameTag>(entity);
        if (tag)
            m_name_index.erase(tag->name);

        m_registry.destroy(entity);
    }

    entt::entity Scene::findEntity(const std::string& name) const
    {
        auto it = m_name_index.find(name);
        return (it != m_name_index.end()) ? it->second : entt::null;
    }

    bool Scene::valid(entt::entity entity) const { return m_registry.valid(entity); }

    std::shared_ptr<SceneNode> Scene::createNode(const std::string& name) { return std::make_shared<SceneNode>(name); }

    std::shared_ptr<SceneNode> Scene::createNodeWithEntity(const std::string& name)
    {
        auto entity = createEntity(name);
        auto node   = std::make_shared<SceneNode>(name);
        node->setEntity(entity);
        return node;
    }

} // namespace RealmEngine
