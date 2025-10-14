#pragma once

#include "render/render_camera.h"
#include "render/render_light.h"
#include "render/render_object.h"
#include <vector>

namespace RealmEngine
{

    class RenderScene
    {
    public:
        RenderScene()           = default;
        ~RenderScene() noexcept = default;

        RenderScene(const RenderScene&)                = delete;
        RenderScene& operator=(const RenderScene&)     = delete;
        RenderScene(RenderScene&&) noexcept            = default;
        RenderScene& operator=(RenderScene&&) noexcept = default;

        void addRenderObject(RenderObject&& obj);
        void clearRenderObjects();
        void sortTransparentObjects(const glm::vec3& camera_pos);
        void cullObjects(const Frustum& frustum);

        void addDirectionalLight(const DirectionalLight& light);
        void addPointLight(const PointLight& light);
        void addSpotLight(const SpotLight& light);
        void setAmbientLight(const AmbientLight& ambient);
        void clearLights();

        const std::vector<RenderObject>&     getOpaqueObjects() const;
        const std::vector<RenderObject>&     getTransparentObjects() const;
        const std::vector<DirectionalLight>& getDirectionalLights() const;
        const std::vector<PointLight>&       getPointLights() const;
        const std::vector<SpotLight>&        getSpotLights() const;
        const AmbientLight&                  getAmbientLight() const;

    private:
        std::vector<RenderObject> m_opaque_objs;
        std::vector<RenderObject> m_transparent_objs;
        std::vector<RenderObject> m_culled_objects;

        std::vector<DirectionalLight> m_dir_lights;
        std::vector<PointLight>       m_point_lights;
        std::vector<SpotLight>        m_spot_lights;
        AmbientLight                  m_ambient_light;
    };
} // namespace RealmEngine