#pragma once

#include "render/render_object.h"
#include <memory>
#include <vector>
namespace RealmEngine
{

    class DirectionalLight;
    class PointLight;
    class SpotLight;
    class Camera;
    class Skybox;

    class RenderScene
    {
    public:
        RenderScene()           = default;
        ~RenderScene() noexcept = default;

        RenderScene(const RenderScene&)                = delete;
        RenderScene& operator=(const RenderScene&)     = delete;
        RenderScene(RenderScene&&) noexcept            = default;
        RenderScene& operator=(RenderScene&&) noexcept = default;

    private:
        std::vector<RenderObject> m_render_objects;

        std::vector<DirectionalLight> m_directional_lights;
        std::vector<PointLight>       m_point_lights;
        std::vector<SpotLight>        m_spot_lights;

        std::weak_ptr<Camera> m_main_camera;
        std::weak_ptr<Skybox> m_skybox;
    };
} // namespace RealmEngine