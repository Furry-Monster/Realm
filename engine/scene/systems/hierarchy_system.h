#pragma once

#include <cstdint>
#include <entt/entity/entity.hpp>
#include <memory>
#include <unordered_map>

namespace RealmEngine
{
    class Scene;
    class SceneNode;

    /**
     * @brief
     * Synchronises SceneNode tree -> ECS Parent / Children components.
     * Call once per frame *before* TransformSystem so hierarchy data is fresh.
     * Skips the rebuild when the scene generation has not changed since last sync.
     */
    class HierarchySystem
    {
    public:
        static void update(Scene& scene);

        // Force a full resync on the next update() call
        static void invalidate(Scene& scene);
        static void invalidateAll();

    private:
        static void syncNode(Scene& scene, const std::shared_ptr<SceneNode>& node, entt::entity parent_entity);

        static std::unordered_map<const Scene*, uint64_t> s_synced_generations;
    };

} // namespace RealmEngine
