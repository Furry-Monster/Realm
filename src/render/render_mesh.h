#pragma once

#include "graphic_res_manager.h"

namespace RealmEngine
{
    class RenderMesh
    {
    public:
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