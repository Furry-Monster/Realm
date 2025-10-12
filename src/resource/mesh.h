#pragma once

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
        glm::vec2 tex_coord; // aka uv coord
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec4 color;
    };

    struct SubMesh
    {
        uint32_t base_index;
        uint32_t index_count;
        uint32_t material_idx; // override the origin material of the mesh

        SubMesh() : base_index(0), index_count(0), material_idx(0) {}
        SubMesh(uint32_t base_idx, uint32_t idx_count, uint32_t mat_idx) :
            base_index(base_idx), index_count(idx_count), material_idx(mat_idx)
        {}

        constexpr uint32_t getTriangleCount() const { return index_count / 3; }
        constexpr uint32_t getEndIndex() const { return base_index + index_count; }
        constexpr bool     isEmpty() const { return index_count == 0; }
        constexpr bool     isValid() const { return index_count > 0 && index_count % 3 == 0; }
    };

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;

        constexpr glm::vec3 center() const { return (min + max) * 0.5f; }
        constexpr glm::vec3 extent() const { return max - min; }
    };

    class Mesh
    {
    public:
        Mesh()           = default;
        ~Mesh() noexcept = default;

        Mesh(const Mesh& that)                = delete;
        Mesh& operator=(const Mesh& that)     = delete;
        Mesh(Mesh&& that) noexcept            = default;
        Mesh& operator=(Mesh&& that) noexcept = default;

        // Getters & Setters
        const std::vector<Vertex>&   getVertices() const;
        const std::vector<uint32_t>& getIndices() const;
        const std::vector<SubMesh>&  getSubMeshes() const;
        const AABB&                  getBounds() const;

        std::vector<Vertex>&   getVertices();
        std::vector<uint32_t>& getIndices();
        std::vector<SubMesh>&  getSubMeshes();
        AABB&                  getAABB();

        void setVertices(std::vector<Vertex>&& vertices);
        void setIndices(std::vector<uint32_t>&& indices);

        // SubMesh
        void addSubMesh(const SubMesh& submesh);
        void clearSubMeshes();

        // Mesh Utilities
        void calculateNormals();
        void calculateTangents();
        void calculateAABB();
        bool isValid() const;
        void clear();

        // GPU data management
        bool isGpuDataDirty() const;
        void markGpuDataSynced();

    private:
        std::vector<Vertex>   m_verts;
        std::vector<uint32_t> m_indices;
        std::vector<SubMesh>  m_submeshes;
        AABB                  m_aabb;

        bool m_gpu_data_dirty {true};
    };
} // namespace RealmEngine