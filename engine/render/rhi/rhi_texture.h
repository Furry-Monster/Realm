#pragma once

#include <cstdint>

#include "render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHITexture
    {
    public:
        virtual ~RHITexture() = default;

        virtual void bind(uint32_t unit = 0)   = 0;
        virtual void unbind(uint32_t unit = 0) = 0;

        virtual void generateMipmaps() = 0;

        // Re-allocate storage (for framebuffer resize, etc.)
        virtual void resize(int width, int height) = 0;

        // Bind as image for compute shader read/write (GL 4.3+)
        virtual void bindImage(uint32_t unit, int level, bool layered, TextureAccess access) = 0;

        int           getWidth() const { return m_width; }
        int           getHeight() const { return m_height; }
        int           getDepth() const { return m_depth; }
        TextureType   getType() const { return m_type; }
        TextureFormat getFormat() const { return m_format; }

        // Backend-specific handle (e.g. GL texture id).
        // Used internally by RHI implementations (framebuffer attachment, etc.).
        // Avoid calling from renderer/application code; prefer RHI abstractions.
        virtual uint32_t getNativeHandle() const = 0;

    protected:
        int           m_width {};
        int           m_height {};
        int           m_depth {};
        TextureType   m_type {};
        TextureFormat m_format {};
    };

} // namespace RealmEngine
