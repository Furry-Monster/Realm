#pragma once

#include <entt/entity/entity.hpp>
#include <memory>

namespace RealmEngine
{
    class Scene;
    class SceneNode;

    /**
     * @brief
     * Synchronises SceneNode tree → ECS Parent / Children components.
     * Call once per frame *before* TransformSystem so hierarchy data is fresh.
     */
    class HierarchySystem
    {
    public:
        static void update(Scene& scene);

    private:
        static void syncNode(Scene& scene, const std::shared_ptr<SceneNode>& node, entt::entity parent_entity);
    };

} // namespace RealmEngine
