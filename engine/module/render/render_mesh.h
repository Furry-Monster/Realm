#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/math/aabb.h"
#include "module/render/material.h"
#include "module/render/rhi/rhi_shader.h"

namespace RealmEngine
{
    class RHIDevice;
    class RHIBuffer;
    class RHIVertexInput;

    struct RenderVertex
    {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec2 m_texture_coordinates;
        glm::vec3 m_tangent;
        glm::vec3 m_bitangent;
    };

    class RenderMesh
    {
    public:
        RenderMesh(std::vector<RenderVertex> vertices,
                   std::vector<unsigned int> indices,
                   Material                  material,
                   RHIDevice&                device,
                   std::string               name = "");
        ~RenderMesh() noexcept;

        RenderMesh(const RenderMesh&)            = delete;
        RenderMesh& operator=(const RenderMesh&) = delete;
        RenderMesh(RenderMesh&& other) noexcept;
        RenderMesh& operator=(RenderMesh&& other) noexcept;

        void draw(RHIShader& shader);
        void drawShadow(RHIShader& shader);

        [[nodiscard]] int  getTriangleCount() const { return static_cast<int>(m_indices.size()) / 3; }
        [[nodiscard]] AABB getLocalAABB() const;

        std::string               m_name;
        std::vector<RenderVertex> m_vertices;
        std::vector<unsigned int> m_indices;
        Material                  m_material;

    private:
        std::unique_ptr<RHIBuffer>      m_vertex_buffer;
        std::unique_ptr<RHIBuffer>      m_index_buffer;
        std::unique_ptr<RHIVertexInput> m_vertex_input;
    };
} // namespace RealmEngine
