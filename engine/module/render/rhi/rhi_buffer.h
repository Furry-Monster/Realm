#pragma once

#include <cstddef>
#include <cstdint>

#include "module/render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIBuffer
    {
    public:
        virtual ~RHIBuffer() noexcept = default;

        virtual void bind()   = 0;
        virtual void unbind() = 0;

        // Upload / update data
        virtual void setData(const void* data, size_t size)                   = 0;
        virtual void setSubData(const void* data, size_t offset, size_t size) = 0;

        // Uniform buffer binding point
        virtual void bindBase(uint32_t binding_point) = 0;

        BufferType  getType() const { return m_type; }
        BufferUsage getUsage() const { return m_usage; }

    protected:
        BufferType  m_type {};
        BufferUsage m_usage {};
    };

} // namespace RealmEngine
