#pragma once

#include <entt/entity/entity.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "renderer/light.h"
#include "renderer/render_object.h"

namespace RealmEngine
{
    class Scene;
    class SceneNode;

    class RenderScene
    {
    public:
        RenderScene()           = default;
        ~RenderScene() noexcept = default;

        RenderScene(const RenderScene&)                = delete;
        RenderScene& operator=(const RenderScene&)     = delete;
        RenderScene(RenderScene&&) noexcept            = default;
        RenderScene& operator=(RenderScene&&) noexcept = default;

        void syncFromScene(const std::shared_ptr<Scene>& scene);

        Scene* getScene() const { return m_cached_scene.get(); }

        int getDrawCallCount() const;
        int getTriangleCount() const;

        const std::vector<std::shared_ptr<RenderObject>>& getRenderObjects() const { return m_render_objects; }
        std::vector<std::shared_ptr<RenderObject>>&       getRenderObjects() { return m_render_objects; }
        const std::vector<glm::mat4>& getRenderModelMatrices() const { return m_render_model_matrices; }
        std::vector<glm::mat4>&       getRenderModelMatrices() { return m_render_model_matrices; }
        const std::vector<Light>&     getLights() const { return m_lights; }
        std::vector<Light>&           getLights() { return m_lights; }

        std::optional<std::reference_wrapper<const Light>> findDirectionalLight() const;

    private:
        void fullSync(Scene& scene);
        void syncNode(Scene& scene, std::shared_ptr<SceneNode> node);
        void updateTransformsOnly(Scene& scene);

        std::vector<std::shared_ptr<RenderObject>> m_render_objects;
        std::vector<glm::mat4>                     m_render_model_matrices;
        std::vector<Light>                         m_lights;

        std::shared_ptr<Scene>    m_cached_scene;
        uint64_t                  m_cached_generation {0};
        std::vector<entt::entity> m_render_entities;
        std::vector<entt::entity> m_light_entities;
    };
} // namespace RealmEngine
