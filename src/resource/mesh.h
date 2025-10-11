#pragma once

#include "render/resource_manager.h"

#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <vector>

namespace RealmEngine
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coord; // uv coord
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec4 color;
    };

    struct SubMesh
    {
        uint32_t vert_idx_start;
        uint32_t vert_idx_end;
        uint32_t material_idx;
    };

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;

        glm::vec3 center() const { return (min + max) * 0.5f; }
        glm::vec3 extent() const { return max - min; }
    };

    class Mesh
    {
    public:
        Mesh()           = default;
        ~Mesh() noexcept = default;

        Mesh(const Mesh& that)            = delete;
        Mesh& operator=(const Mesh& that) = delete;
        Mesh(Mesh&& that) noexcept;
        Mesh& operator=(Mesh&& that) noexcept;

        const std::vector<Vertex>&   getVertices() const;
        const std::vector<uint32_t>& getIndices() const;
        const std::vector<SubMesh>&  getSubMeshes() const;
        const AABB&                  getBounds() const;

        std::vector<Vertex>&   getVertices();
        std::vector<uint32_t>& getIndices();
        std::vector<SubMesh>&  getSubMeshes();

        void setVertices(std::vector<Vertex>&& vertices);
        void setIndices(std::vector<uint32_t>&& indices);
        void addSubMesh(const SubMesh& submesh);

        void calculateNormals();
        void calculateTangents();
        void calculateBounds();
        bool isValid() const;

        VAOHandle    getVAO() const;
        BufferHandle getVBO() const;
        BufferHandle getEBO() const;

        void setVAO(VAOHandle vao);
        void setVBO(BufferHandle vbo);
        void setEBO(BufferHandle ebo);

    private:
        std::vector<Vertex>   m_verts;
        std::vector<uint32_t> m_indices;
        std::vector<SubMesh>  m_submeshes;
        AABB                  m_aabb;

        VAOHandle    m_vao {0};
        BufferHandle m_vbo {0};
        BufferHandle m_ebo {0};
        bool         m_gpu_data_dirty {true};
    };
} // namespace RealmEngine