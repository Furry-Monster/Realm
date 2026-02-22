#pragma once

#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "functional/render/light.h"
#include "functional/render/render_object.h"

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
        std::vector<std::shared_ptr<RenderObject>> m_render_objects;
        std::vector<glm::mat4>                     m_render_model_matrices;
        std::vector<Light>                         m_lights;
    };
} // namespace RealmEngine
