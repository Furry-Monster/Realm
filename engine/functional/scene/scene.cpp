#include "functional/scene/scene.h"

#include "core/base/macros.h"
#include "module/ecs/components/lighting/area.h"
#include "module/ecs/components/lighting/directional.h"
#include "module/ecs/components/lighting/point.h"
#include "module/ecs/components/lighting/spot.h"
#include "module/ecs/components/name_tag.h"
#include "module/ecs/components/renderable.h"
#include "module/ecs/components/scene_view_camera_controller.h"
#include "functional/ecs/systems/hierarchy_system.h"
#include "functional/ecs/systems/transform_system.h"

namespace RealmEngine
{
    Scene::Scene()
    {
        m_root                         = std::make_shared<SceneNode>("Root");
        m_scene_view_camera_controller = std::make_shared<SceneViewCameraController>();

        m_registry.on_construct<Renderable>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<PointLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<SpotLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<AreaLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<DirectionalLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<Renderable>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<PointLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<SpotLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<AreaLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<DirectionalLight>().connect<&Scene::onRenderStructureChanged>(this);
    }

    void Scene::reconnectSignals()
    {
        m_registry.on_construct<Renderable>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<PointLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<SpotLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<AreaLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_construct<DirectionalLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<Renderable>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<PointLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<SpotLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<AreaLight>().connect<&Scene::onRenderStructureChanged>(this);
        m_registry.on_destroy<DirectionalLight>().connect<&Scene::onRenderStructureChanged>(this);
    }

    Scene::Scene(Scene&& other) noexcept :
        m_registry(std::move(other.m_registry)), m_name_index(std::move(other.m_name_index)),
        m_root(std::move(other.m_root)),
        m_scene_view_camera_controller(std::move(other.m_scene_view_camera_controller)),
        m_generation(other.m_generation)
    {
        reconnectSignals();
    }

    Scene& Scene::operator=(Scene&& other) noexcept
    {
        if (this != &other)
        {
            m_registry                     = std::move(other.m_registry);
            m_name_index                   = std::move(other.m_name_index);
            m_root                         = std::move(other.m_root);
            m_scene_view_camera_controller = std::move(other.m_scene_view_camera_controller);
            m_generation                   = other.m_generation;
            reconnectSignals();
        }
        return *this;
    }

    void Scene::tick([[maybe_unused]] const float delta_time)
    {
        // Run ECS systems each frame
        HierarchySystem::update(*this);
        TransformSystem::update(*this);
    }

    Entity Scene::createEntity(std::string name)
    {
        const auto it = m_name_index.find(name);
        if (it != m_name_index.end())
        {
            RE_LOG_WARN("Entity name already taken: " + name + " -- generating unique name");
            const std::string base   = std::move(name);
            int               suffix = 1;
            std::string       unique_name;
            do
            {
                unique_name = base + "_" + std::to_string(suffix++);
            } while (m_name_index.count(unique_name));
            return createEntity(std::move(unique_name));
        }

        const auto  handle     = m_registry.create();
        const auto& tag        = m_registry.emplace<NameTag>(handle, NameTag {std::move(name)});
        m_name_index[tag.name] = handle;
        incrementGeneration();
        return Entity(handle, &m_registry);
    }

    void Scene::destroyEntity(entt::entity entity)
    {
        const auto* tag = m_registry.try_get<NameTag>(entity);
        if (tag)
            m_name_index.erase(tag->name);

        const auto node = findNodeByEntity(entity);
        if (node)
        {
            const auto parent = node->getParent();
            if (parent)
                parent->removeChild(node);
            node->clearChildren();
        }

        m_registry.destroy(entity);
        incrementGeneration();
    }

    Entity Scene::findEntity(const std::string& name) const
    {
        const auto it = m_name_index.find(name);
        if (it != m_name_index.end())
            return Entity(it->second, const_cast<entt::registry*>(&m_registry));
        return Entity();
    }

    bool Scene::valid(const entt::entity entity) const { return m_registry.valid(entity); }

    namespace
    {
        std::shared_ptr<SceneNode> findNodeByEntityRecursive(std::shared_ptr<SceneNode> node, entt::entity entity)
        {
            if (!node)
                return nullptr;
            if (node->hasEntity() && node->getEntity() == entity)
                return node;
            for (size_t i = 0; i < node->getChildCount(); ++i)
            {
                const auto found = findNodeByEntityRecursive(node->getChild(i), entity);
                if (found)
                    return found;
            }
            return nullptr;
        }
    } // namespace

    std::shared_ptr<SceneNode> Scene::findNodeByEntity(entt::entity entity) const
    {
        return findNodeByEntityRecursive(m_root, entity);
    }

    void Scene::onRenderStructureChanged(entt::registry&, entt::entity) { incrementGeneration(); }

    std::shared_ptr<SceneNode> Scene::createNode(const std::string& name) { return std::make_shared<SceneNode>(name); }

    std::shared_ptr<SceneNode> Scene::createNodeWithEntity(const std::string& name)
    {
        const auto entity = createEntity(name);
        auto       node   = std::make_shared<SceneNode>(name);
        node->setEntity(entity.handle());
        return node;
    }

} // namespace RealmEngine
