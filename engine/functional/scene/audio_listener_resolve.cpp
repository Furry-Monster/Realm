#include "functional/scene/audio_listener_resolve.h"

#include <glm/gtc/quaternion.hpp>

#include "module/ecs/components/audio/audio_listener.h"
#include "module/ecs/components/transform.h"
#include "module/ecs/components/world_transform.h"
#include "functional/scene/scene.h"

namespace RealmEngine
{
    entt::entity findPrimaryAudioListenerEntity(const Scene& scene)
    {
        const auto view = scene.getRegistry().view<AudioListener, WorldTransform>();
        for (const auto entity : view)
        {
            if (view.get<AudioListener>(entity).primary)
                return entity;
        }
        const auto fallback = scene.getRegistry().view<AudioListener, Transform>();
        for (const auto entity : fallback)
        {
            if (fallback.get<AudioListener>(entity).primary)
                return entity;
        }
        return entt::null;
    }

    ListenerPose getListenerPoseFromEntity(const Scene& scene, entt::entity entity)
    {
        ListenerPose pose {};
        pose.forward = {0.0f, 0.0f, -1.0f};
        pose.up      = {0.0f, 1.0f, 0.0f};

        if (const auto* wt = scene.tryGet<WorldTransform>(entity))
        {
            const glm::mat4& m = wt->matrix;
            pose.position      = glm::vec3(m[3]);
            const glm::quat q  = glm::quat_cast(glm::mat3(m));
            pose.forward       = q * glm::vec3(0.0f, 0.0f, -1.0f);
            pose.up            = q * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (const auto* t = scene.tryGet<Transform>(entity))
        {
            pose.position = t->position;
            pose.forward  = t->getForward();
            pose.up       = t->getUp();
        }
        return pose;
    }

} // namespace RealmEngine
