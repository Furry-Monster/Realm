#include "functional/ecs/systems/transform_system.h"

#include "functional/ecs/components/hierarchy.h"
#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/scene/scene.h"

namespace RealmEngine
{
    void TransformSystem::update(Scene& scene)
    {
        auto& registry = scene.getRegistry();

        const auto view = registry.view<Transform>(entt::exclude<Parent>);
        for (const auto entity : view)
        {
            updateEntity(scene, entity, glm::mat4(1.0f));
        }
    }

    void TransformSystem::updateEntity(Scene& scene, const entt::entity entity, const glm::mat4& parent_world)
    {
        auto& registry = scene.getRegistry();

        const auto* transform = registry.try_get<Transform>(entity);
        if (!transform)
            return;

        const glm::mat4 local_matrix = transform->getModelMatrix();
        const glm::mat4 world_matrix = parent_world * local_matrix;

        registry.emplace_or_replace<WorldTransform>(entity, WorldTransform {world_matrix});

        const auto* children = registry.try_get<Children>(entity);
        if (children)
        {
            for (const auto child : children->entities)
            {
                updateEntity(scene, child, world_matrix);
            }
        }
    }

} // namespace RealmEngine
