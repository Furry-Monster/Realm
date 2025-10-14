#pragma once

#include "render/rhi.h"
#include <cstdint>

namespace RealmEngine
{
    class Mesh;

    class RenderMesh
    {
    public:
        RenderMesh()           = default;
        ~RenderMesh() noexcept = default;

        RenderMesh(const RenderMesh& that)                = delete;
        RenderMesh& operator=(const RenderMesh& that)     = delete;
        RenderMesh(RenderMesh&& that) noexcept            = default;
        RenderMesh& operator=(RenderMesh&& that) noexcept = default;

        VAOHandle    getVAO() const;
        BufferHandle getVBO() const;
        BufferHandle getEBO() const;
        uint32_t     getIndexCount() const;
        uint32_t     getVertexCount() const;

        void sync(RHI& rhi, const Mesh& mesh);
        void disposal(RHI& rhi);

        void bind() const;
        void unbind() const;

    private:
        VAOHandle    m_vao {0};
        BufferHandle m_vbo {0};
        BufferHandle m_ebo {0};
        uint32_t     m_index_count {0};
        uint32_t     m_vertex_count {0};
    };
} // namespace RealmEngine