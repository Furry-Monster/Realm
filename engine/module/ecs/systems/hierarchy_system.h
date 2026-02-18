#pragma once

#include <entt/entity/entity.hpp>
#include <memory>
#include <unordered_map>

namespace RealmEngine
{
    class Scene;
    class SceneNode;

    class HierarchySystem
    {
    public:
        static void update(Scene& scene);

        static void invalidate(const Scene& scene);
        static void invalidateAll();

    private:
        static void syncNode(Scene& scene, const std::shared_ptr<SceneNode>& node, entt::entity parent_entity);

        static std::unordered_map<const Scene*, uint64_t> s_synced_generations;
    };

} // namespace RealmEngine
