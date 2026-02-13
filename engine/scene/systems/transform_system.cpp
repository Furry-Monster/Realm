#include "scene/systems/transform_system.h"

#include "scene/components/hierarchy.h"
#include "scene/components/transform.h"
#include "scene/components/world_transform.h"
#include "scene/scene.h"

namespace RealmEngine
{
    void TransformSystem::update(Scene& scene)
    {
        auto& registry = scene.getRegistry();

        // Process root entities: have Transform but no Parent
        auto view = registry.view<Transform>(entt::exclude<Parent>);
        for (auto entity : view)
        {
            updateEntity(scene, entity, glm::mat4(1.0f));
        }
    }

    void TransformSystem::updateEntity(Scene& scene, entt::entity entity, const glm::mat4& parent_world)
    {
        auto& registry = scene.getRegistry();

        auto* transform = registry.try_get<Transform>(entity);
        if (!transform)
            return;

        glm::mat4 local_matrix = transform->getModelMatrix();
        glm::mat4 world_matrix = parent_world * local_matrix;

        // Store computed world matrix
        registry.emplace_or_replace<WorldTransform>(entity, WorldTransform {world_matrix});

        // Recurse into children
        auto* children = registry.try_get<Children>(entity);
        if (children)
        {
            for (auto child : children->entities)
            {
                updateEntity(scene, child, world_matrix);
            }
        }
    }

} // namespace RealmEngine
