#pragma once

#include <cstdint>

#include "render/rhi/rhi_buffer.h"

namespace RealmEngine
{
    class GLBuffer final : public RHIBuffer
    {
    public:
        GLBuffer(BufferType type, BufferUsage usage, const void* data, size_t size);
        ~GLBuffer() override;

        GLBuffer(const GLBuffer&)            = delete;
        GLBuffer& operator=(const GLBuffer&) = delete;

        void bind() override;
        void unbind() override;
        void setData(const void* data, size_t size) override;
        void setSubData(const void* data, size_t offset, size_t size) override;
        void bindBase(uint32_t binding_point) override;

        uint32_t getNativeHandle() const { return m_id; }

    private:
        uint32_t m_id {0};
        uint32_t m_target {0}; // GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER
    };

} // namespace RealmEngine
