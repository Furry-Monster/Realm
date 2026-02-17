#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "rhi/rhi_framebuffer.h"

namespace RealmEngine
{
    class GLTexture;

    class GLFramebuffer final : public RHIFramebuffer
    {
    public:
        explicit GLFramebuffer(const FramebufferDesc& desc);
        ~GLFramebuffer() override;

        GLFramebuffer(const GLFramebuffer&)            = delete;
        GLFramebuffer& operator=(const GLFramebuffer&) = delete;

        void bind() override;
        void unbind() override;
        void resize(int width, int height) override;

        RHITexture* getColorAttachment(uint32_t index = 0) override;
        RHITexture* getDepthAttachment() override;

        void setMipLevel(int level) override;
        void setCubeFace(int face) override;
        void setLayer(int layer) override;

        uint32_t getNativeHandle() const override { return m_fbo; }

    private:
        void create();
        void destroy();
        void updateColorAttachments();

        uint32_t                                m_fbo {0};
        uint32_t                                m_depth_rbo {0}; // only when depth is renderbuffer
        std::vector<std::unique_ptr<GLTexture>> m_color_textures;
        std::unique_ptr<GLTexture>              m_depth_texture;
        FramebufferDesc                         m_desc;
        int                                     m_mip_level {0};
        int                                     m_cube_face {0};
        int                                     m_layer {-1}; // -1 = not an array layer
    };

} // namespace RealmEngine
