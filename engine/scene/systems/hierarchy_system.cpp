#include "scene/systems/hierarchy_system.h"

#include "scene/components/hierarchy.h"
#include "scene/scene.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    std::unordered_map<const Scene*, uint64_t> HierarchySystem::s_synced_generations;

    void HierarchySystem::update(Scene& scene)
    {
        uint64_t current_gen = scene.getGeneration();
        auto     it          = s_synced_generations.find(&scene);
        if (it != s_synced_generations.end() && it->second == current_gen)
            return;
        s_synced_generations[&scene] = current_gen;

        auto& registry = scene.getRegistry();

        // Clear stale hierarchy components
        registry.clear<Parent>();
        registry.clear<Children>();

        // Rebuild from SceneNode tree
        auto root = scene.getRoot();
        if (!root)
            return;

        syncNode(scene, root, entt::null);
    }

    void HierarchySystem::invalidate(Scene& scene) { s_synced_generations.erase(&scene); }

    void HierarchySystem::invalidateAll() { s_synced_generations.clear(); }

    void HierarchySystem::syncNode(Scene& scene, const std::shared_ptr<SceneNode>& node, entt::entity parent_entity)
    {
        auto& registry = scene.getRegistry();

        entt::entity current_entity = entt::null;

        if (node->hasEntity())
        {
            current_entity = node->getEntity();

            // Set parent relationship
            if (parent_entity != entt::null)
                registry.emplace<Parent>(current_entity, Parent {parent_entity});

            // Collect direct child entities
            std::vector<entt::entity> child_entities;
            child_entities.reserve(node->getChildCount());

            node->forEachChild([&](const std::shared_ptr<SceneNode>& child) {
                if (child->hasEntity())
                    child_entities.push_back(child->getEntity());
            });

            if (!child_entities.empty())
                registry.emplace<Children>(current_entity, Children {std::move(child_entities)});
        }

        // Recurse into children -- pass current_entity as parent (or forward parent_entity if this node has no entity)
        entt::entity effective_parent = (current_entity != entt::null) ? current_entity : parent_entity;
        node->forEachChild([&](const std::shared_ptr<SceneNode>& child) { syncNode(scene, child, effective_parent); });
    }

} // namespace RealmEngine
