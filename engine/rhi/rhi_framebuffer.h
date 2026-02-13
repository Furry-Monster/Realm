#pragma once

#include <cstdint>

#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHITexture;

    class RHIFramebuffer
    {
    public:
        virtual ~RHIFramebuffer() = default;

        virtual void bind()   = 0;
        virtual void unbind() = 0;

        virtual void resize(int width, int height) = 0;

        // Access attached textures (returns nullptr if index is out of range or attachment is a renderbuffer)
        virtual RHITexture* getColorAttachment(uint32_t index = 0) = 0;
        virtual RHITexture* getDepthAttachment()                   = 0;

        // Select which mip level to render into (for bloom-style multi-resolution rendering)
        virtual void setMipLevel(int level) = 0;

        // Select which cubemap face to render into (0-5 for +X,-X,+Y,-Y,+Z,-Z). No-op for non-cubemap FBOs.
        virtual void setCubeFace(int face) = 0;

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    protected:
        int m_width {};
        int m_height {};
    };

} // namespace RealmEngine
