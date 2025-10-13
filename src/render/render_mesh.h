#pragma once

#include "graphic_res_manager.h"

namespace RealmEngine
{
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

        void setVAO(VAOHandle vao);
        void setVBO(BufferHandle vbo);
        void setEBO(BufferHandle ebo);

    private:
        VAOHandle    m_vao {0};
        BufferHandle m_vbo {0};
        BufferHandle m_ebo {0};
    };
} // namespace RealmEngine