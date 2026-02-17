#pragma once

#include <cstdint>

#include "rhi/rhi_vertex_input.h"

namespace RealmEngine
{
    class RHIBuffer;

    class GLVertexInput final : public RHIVertexInput
    {
    public:
        GLVertexInput(const VertexLayout& layout, RHIBuffer& vertex_buffer, RHIBuffer* index_buffer);
        ~GLVertexInput() override;

        GLVertexInput(const GLVertexInput&)            = delete;
        GLVertexInput& operator=(const GLVertexInput&) = delete;

        void bind() override;
        void unbind() override;

        void draw(PrimitiveType primitive, uint32_t count) override;
        void
        drawIndexed(PrimitiveType primitive, uint32_t index_count, IndexType idx_type = IndexType::UInt32) override;
        void drawInstanced(PrimitiveType primitive, uint32_t count, uint32_t instance_count) override;
        void drawIndexedInstanced(PrimitiveType primitive,
                                  uint32_t      index_count,
                                  uint32_t      instance_count,
                                  IndexType     idx_type = IndexType::UInt32) override;

    private:
        uint32_t    m_vao {0};
        bool        m_has_index_buffer {false};
        RHIBuffer*  m_vertex_buffer {nullptr};
        RHIBuffer*  m_index_buffer {nullptr};
    };

} // namespace RealmEngine
