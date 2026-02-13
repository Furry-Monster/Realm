#pragma once

#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>

namespace RealmEngine
{
    class Scene;

    /**
     * @brief
     * Computes WorldTransform for every entity that owns a Transform component.
     * Uses Parent / Children hierarchy (populated by HierarchySystem) to propagate
     * parent transforms down to children.
     * Call once per frame *after* HierarchySystem::update().
     */
    class TransformSystem
    {
    public:
        static void update(Scene& scene);

    private:
        static void updateEntity(Scene& scene, entt::entity entity, const glm::mat4& parent_world);
    };

} // namespace RealmEngine
