#pragma once

#include "render/render_camera.h"
#include "render/render_light.h"
#include "render/render_object.h"
#include <memory>
#include <vector>
namespace RealmEngine
{
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

        void addRenderObject();
        void removeRenderObject();
        void sortTransparents();
        void cullObjects();
        void updateLightingData();

    private:
        // renderables
        std::vector<RenderObject> m_opaque_objs;
        std::vector<RenderObject> m_transparent_objs;

        // lighs
        std::vector<DirectionalLight> m_dir_lights;
        std::vector<PointLight>       m_point_lights;
        std::vector<SpotLight>        m_spot_lights;

        // misc
        std::weak_ptr<RenderCamera> m_main_camera;
        std::weak_ptr<Skybox>       m_skybox;
    };
} // namespace RealmEngine