#pragma once

#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>

namespace RealmEngine
{
    class Scene;

    struct ListenerPose
    {
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
    };

    entt::entity findPrimaryAudioListenerEntity(const Scene& scene);
    ListenerPose getListenerPoseFromEntity(const Scene& scene, entt::entity entity);

} // namespace RealmEngine
