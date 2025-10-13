#include "render_scene.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "render/render_object.h"
#include <algorithm>
#include <utility>

namespace RealmEngine
{

    void RenderScene::addRenderObject(RenderObject&& obj) { m_opaque_objs.push_back(std::move(obj)); }
    void RenderScene::clearRenderObjects()
    {
        m_opaque_objs.clear();
        m_transparent_objs.clear();
        m_culled_objects.clear();
    }
    void RenderScene::sortTransparentObjects(const glm::vec3& camera_pos)
    {
        std::sort(m_transparent_objs.begin(),
                  m_transparent_objs.end(),
                  [&camera_pos](const RenderObject& a, const RenderObject& b) {
                      glm::vec3 center_a = a.getWorldBounds().center();
                      glm::vec3 center_b = b.getWorldBounds().center();

                      float dist_a = glm::length(center_a - camera_pos);
                      float dist_b = glm::length(center_b - camera_pos);

                      return dist_a > dist_b;
                  });
    }
    void RenderScene::cullObjects(const Frustum& frustum)
    {
        m_culled_objects.clear();

        for (auto& obj : m_opaque_objs)
        {
            if (!obj.isVisible())
                continue;

            if (frustum.containsAABB(obj.getWorldBounds()))
                m_culled_objects.push_back(std::move(obj));
        }

        m_opaque_objs = std::move(m_culled_objects);
    }

    void RenderScene::addDirectionalLight(const DirectionalLight& light) { m_dir_lights.push_back(light); }
    void RenderScene::addPointLight(const PointLight& light) { m_point_lights.push_back(light); }
    void RenderScene::addSpotLight(const SpotLight& light) { m_spot_lights.push_back(light); }
    void RenderScene::setAmbientLight(const AmbientLight& ambient) { m_ambient_light = ambient; }
    void RenderScene::clearLights()
    {
        m_dir_lights.clear();
        m_point_lights.clear();
        m_spot_lights.clear();
    }

    void RenderScene::setMainCamera(std::shared_ptr<RenderCamera> camera) { m_main_camera = camera; }
    void RenderScene::setSkybox(std::shared_ptr<Skybox> skybox) { m_skybox = skybox; }

    const std::vector<RenderObject>&     RenderScene::getOpaqueObjects() const { return m_opaque_objs; }
    const std::vector<RenderObject>&     RenderScene::getTransparentObjects() const { return m_transparent_objs; }
    const std::vector<DirectionalLight>& RenderScene::getDirectionalLights() const { return m_dir_lights; }
    const std::vector<PointLight>&       RenderScene::getPointLights() const { return m_point_lights; }
    const std::vector<SpotLight>&        RenderScene::getSpotLights() const { return m_spot_lights; }
    const AmbientLight&                  RenderScene::getAmbientLight() const { return m_ambient_light; }
    std::shared_ptr<RenderCamera>        RenderScene::getMainCamera() const { return m_main_camera.lock(); }
    std::shared_ptr<Skybox>              RenderScene::getSkybox() const { return m_skybox.lock(); }

} // namespace RealmEngine
