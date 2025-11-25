#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "render/render_object.h"

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

        std::vector<RenderObject> m_objects;
        std::vector<glm::vec3>    m_light_positions;
        std::vector<glm::vec3>    m_light_colors;
    };
} // namespace RealmEngine
