#pragma once

#include <cstdint>

#include "rhi/rhi_types.h"

namespace RealmEngine
{
    // Abstracts the concept of a configured vertex input (OpenGL VAO, Vulkan pipeline vertex state).
    // Holds vertex + index buffer bindings and the vertex attribute layout.
    class RHIVertexInput
    {
    public:
        virtual ~RHIVertexInput() = default;

        virtual void bind()   = 0;
        virtual void unbind() = 0;

        // Draw the bound geometry
        virtual void draw(PrimitiveType primitive, uint32_t count) = 0;
        virtual void
        drawIndexed(PrimitiveType primitive, uint32_t index_count, IndexType idx_type = IndexType::UInt32) = 0;
        virtual void drawInstanced(PrimitiveType primitive, uint32_t count, uint32_t instance_count)       = 0;
        virtual void drawIndexedInstanced(PrimitiveType primitive,
                                          uint32_t      index_count,
                                          uint32_t      instance_count,
                                          IndexType     idx_type = IndexType::UInt32)                          = 0;
    };

} // namespace RealmEngine
