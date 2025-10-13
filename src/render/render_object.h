#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "math.h"
#include "render/render_material.h"
#include "render/render_mesh.h"
#include <memory>

namespace RealmEngine
{
    class RenderObject
    {
    public:
        RenderObject()           = default;
        ~RenderObject() noexcept = default;

        RenderObject(const RenderObject&)                = delete;
        RenderObject& operator=(const RenderObject&)     = delete;
        RenderObject(RenderObject&&) noexcept            = default;
        RenderObject& operator=(RenderObject&&) noexcept = default;

    private:
        glm::mat4 m_model_matrix;
        glm::mat4 m_normal_matrix;

        std::shared_ptr<RenderMesh>     m_mesh;
        std::shared_ptr<RenderMaterial> m_material;

        AABB m_bounds;
    };
} // namespace RealmEngine