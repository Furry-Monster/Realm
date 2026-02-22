#include "module/ecs/systems/hierarchy_system.h"

#include "module/ecs/components/hierarchy.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_node.h"

namespace RealmEngine
{
    std::unordered_map<const Scene*, uint64_t> HierarchySystem::s_synced_generations;

    void HierarchySystem::update(Scene& scene)
    {
        const uint64_t current_gen = scene.getGeneration();
        const auto     it          = s_synced_generations.find(&scene);
        if (it != s_synced_generations.end() && it->second == current_gen)
            return;
        s_synced_generations[&scene] = current_gen;

        auto& registry = scene.getRegistry();

        registry.clear<Parent>();
        registry.clear<Children>();

        const auto root = scene.getRoot();
        if (!root)
            return;

        syncNode(scene, root, entt::null);
    }

    void HierarchySystem::invalidate(const Scene& scene) { s_synced_generations.erase(&scene); }

    void HierarchySystem::invalidateAll() { s_synced_generations.clear(); }

    void
    HierarchySystem::syncNode(Scene& scene, const std::shared_ptr<SceneNode>& node, const entt::entity parent_entity)
    {
        auto& registry = scene.getRegistry();

        entt::entity current_entity = entt::null;

        if (node->hasEntity())
        {
            current_entity = node->getEntity();

            if (parent_entity != entt::null)
                registry.emplace<Parent>(current_entity, Parent {parent_entity});

            std::vector<entt::entity> child_entities;
            child_entities.reserve(node->getChildCount());

            node->forEachChild([&](const std::shared_ptr<SceneNode>& child) {
                if (child->hasEntity())
                    child_entities.push_back(child->getEntity());
            });

            if (!child_entities.empty())
                registry.emplace<Children>(current_entity, Children {std::move(child_entities)});
        }

        const entt::entity effective_parent = (current_entity != entt::null) ? current_entity : parent_entity;
        node->forEachChild([&](const std::shared_ptr<SceneNode>& child) { syncNode(scene, child, effective_parent); });
    }

} // namespace RealmEngine
