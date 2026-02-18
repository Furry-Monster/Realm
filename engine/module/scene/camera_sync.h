#pragma once

#include <entt/entity/entity.hpp>

namespace RealmEngine
{
    class Scene;
    class RenderCamera;

    entt::entity findPrimaryCameraEntity(const Scene& scene);

    void syncEntityCameraToRenderCamera(const Scene&  scene,
                                        entt::entity  camera_entity,
                                        RenderCamera& render_camera,
                                        float         aspect_ratio);

} // namespace RealmEngine
