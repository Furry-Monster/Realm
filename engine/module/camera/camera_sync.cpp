#include "module/camera/camera_sync.h"

#include <glm/gtc/quaternion.hpp>

#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/render/render_camera.h"
#include "functional/scene/scene.h"
#include "module/camera/components/camera.h"

namespace RealmEngine
{
    entt::entity findPrimaryCameraEntity(const Scene& scene)
    {
        const auto view = scene.getRegistry().view<Camera, WorldTransform>();
        for (const auto entity : view)
        {
            const auto& cam = view.get<Camera>(entity);
            if (cam.primary)
                return entity;
        }
        const auto fallback = scene.getRegistry().view<Camera, Transform>();
        for (const auto entity : fallback)
        {
            const auto& cam = fallback.get<Camera>(entity);
            if (cam.primary)
                return entity;
        }
        return entt::null;
    }

    void syncEntityCameraToRenderCamera(const Scene&       scene,
                                        const entt::entity camera_entity,
                                        RenderCamera&      render_camera,
                                        const float        aspect_ratio)
    {
        if (!scene.valid(camera_entity))
            return;

        const auto* cam = scene.tryGet<Camera>(camera_entity);
        if (!cam)
            return;

        glm::vec3 position;
        glm::quat rotation;

        if (const auto* wt = scene.tryGet<WorldTransform>(camera_entity))
        {
            const glm::mat4& m = wt->matrix;
            position           = glm::vec3(m[3]);
            rotation           = glm::quat_cast(glm::mat3(m));
        }
        else if (const auto* tf = scene.tryGet<Transform>(camera_entity))
        {
            position = tf->position;
            rotation = tf->rotation;
        }
        else
        {
            return;
        }

        render_camera.setPosition(position);
        render_camera.setRotation(rotation);
        render_camera.setAspectRatio(aspect_ratio);

        if (cam->projection_type == CameraProjectionType::Perspective)
        {
            render_camera.setPerspective(cam->fov, aspect_ratio, cam->near_plane, cam->far_plane);
        }
        else
        {
            render_camera.setOrthographic(
                cam->ortho_left, cam->ortho_right, cam->ortho_bottom, cam->ortho_top, cam->near_plane, cam->far_plane);
        }
        render_camera.update();
    }
} // namespace RealmEngine
