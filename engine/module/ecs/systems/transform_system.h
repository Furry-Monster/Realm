#pragma once

#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>

namespace RealmEngine
{
    class Scene;

    class TransformSystem
    {
    public:
        static void update(Scene& scene);

    private:
        static void updateEntity(Scene& scene, entt::entity entity, const glm::mat4& parent_world);
    };

} // namespace RealmEngine
