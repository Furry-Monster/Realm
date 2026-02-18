#include "render/rhi/opengl/gl_framebuffer.h"

#include <glad/glad.h>
#include <algorithm>

#include "core/base/macros.h"
#include "render/rhi/opengl/gl_format_utils.h"
#include "render/rhi/opengl/gl_texture.h"

namespace RealmEngine
{
    static GLenum depthAttachmentType(TextureFormat fmt)
    {
        return (fmt == TextureFormat::Depth24Stencil8) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
    }

    GLFramebuffer::GLFramebuffer(const FramebufferDesc& desc) : m_desc(desc)
    {
        m_width  = desc.width;
        m_height = desc.height;
        create();
    }

    GLFramebuffer::~GLFramebuffer() noexcept { destroy(); }

    void GLFramebuffer::create()
    {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // Color attachments
        std::vector<GLenum> draw_buffers;
        for (uint32_t i = 0; i < m_desc.color_attachments.size(); ++i)
        {
            auto& att = m_desc.color_attachments[i];

            TextureType tex_type = TextureType::Texture2D;
            if (att.is_cubemap)
                tex_type = TextureType::TextureCube;
            else if (att.is_array)
                tex_type = TextureType::Texture2DArray;

            TextureDesc td;
            td.type       = tex_type;
            td.format     = att.format;
            td.width      = m_width;
            td.height     = m_height;
            td.depth      = att.layers;
            td.min_filter = att.min_filter;
            td.mag_filter = att.mag_filter;
            td.wrap_s     = att.wrap;
            td.wrap_t     = att.wrap;
            td.wrap_r     = att.wrap;
            td.gen_mips   = att.gen_mips;

            auto tex = std::make_unique<GLTexture>(td);
            if (att.is_array)
            {
                // Attach first layer by default; use setLayer() to change
                glFramebufferTextureLayer(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex->getNativeHandle(), m_mip_level, 0);
            }
            else
            {
                GLenum attach_target =
                    att.is_cubemap ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_cube_face) : GL_TEXTURE_2D;
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attach_target, tex->getNativeHandle(), m_mip_level);
            }
            m_color_textures.push_back(std::move(tex));
            draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        if (!draw_buffers.empty())
            glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());
        else
            glDrawBuffer(GL_NONE); // depth-only FBO

        // Depth attachment
        if (m_desc.has_depth)
        {
            auto& datt = m_desc.depth_attachment;
            if (datt.is_renderbuffer)
            {
                GLenum internal_fmt = toGLInternalFormat(datt.format);
                glGenRenderbuffers(1, &m_depth_rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, m_depth_rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, internal_fmt, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER, depthAttachmentType(datt.format), GL_RENDERBUFFER, m_depth_rbo);
            }
            else
            {
                TextureDesc td;
                td.type       = datt.is_array ? TextureType::Texture2DArray : TextureType::Texture2D;
                td.format     = datt.format;
                td.width      = m_width;
                td.height     = m_height;
                td.depth      = datt.layers;
                td.min_filter = datt.min_filter;
                td.mag_filter = datt.mag_filter;
                td.wrap_s     = datt.wrap;
                td.wrap_t     = datt.wrap;

                m_depth_texture = std::make_unique<GLTexture>(td);
                if (datt.is_array)
                {
                    // Attach whole texture; use setLayer() to select specific layer for rendering
                    glFramebufferTextureLayer(
                        GL_FRAMEBUFFER, depthAttachmentType(datt.format), m_depth_texture->getNativeHandle(), 0, 0);
                }
                else
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER,
                                           depthAttachmentType(datt.format),
                                           GL_TEXTURE_2D,
                                           m_depth_texture->getNativeHandle(),
                                           0);
                }
            }
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            RE_LOG_FATAL("GLFramebuffer: framebuffer not complete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::destroy()
    {
        m_color_textures.clear();
        m_depth_texture.reset();

        if (m_depth_rbo != 0)
        {
            glDeleteRenderbuffers(1, &m_depth_rbo);
            m_depth_rbo = 0;
        }
        if (m_fbo != 0)
        {
            glDeleteFramebuffers(1, &m_fbo);
            m_fbo = 0;
        }
    }

    void GLFramebuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // Compute mip-level viewport
        int w = std::max(1, m_width >> m_mip_level);
        int h = std::max(1, m_height >> m_mip_level);
        glViewport(0, 0, w, h);
    }

    void GLFramebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void GLFramebuffer::resize(int width, int height)
    {
        m_width       = width;
        m_height      = height;
        m_desc.width  = width;
        m_desc.height = height;
        destroy();
        create();
    }

    RHITexture* GLFramebuffer::getColorAttachment(uint32_t index)
    {
        return (index < m_color_textures.size()) ? m_color_textures[index].get() : nullptr;
    }

    RHITexture* GLFramebuffer::getDepthAttachment() { return m_depth_texture.get(); }

    void GLFramebuffer::setMipLevel(int level)
    {
        m_mip_level = level;

        // Resize depth renderbuffer when mip level changes (for cubemap mip-chain rendering)
        if (m_desc.has_depth && m_desc.depth_attachment.is_renderbuffer && m_depth_rbo != 0)
        {
            int    w            = std::max(1, m_width >> m_mip_level);
            int    h            = std::max(1, m_height >> m_mip_level);
            GLenum internal_fmt = toGLInternalFormat(m_desc.depth_attachment.format);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, internal_fmt, w, h);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        updateColorAttachments();
    }

    void GLFramebuffer::setCubeFace(int face)
    {
        m_cube_face = face;
        updateColorAttachments();
    }

    void GLFramebuffer::setLayer(int layer)
    {
        m_layer = layer;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // Re-attach color textures at specified layer
        for (uint32_t i = 0; i < m_color_textures.size(); ++i)
        {
            if (i < m_desc.color_attachments.size() && m_desc.color_attachments[i].is_array)
            {
                glFramebufferTextureLayer(GL_FRAMEBUFFER,
                                          GL_COLOR_ATTACHMENT0 + i,
                                          m_color_textures[i]->getNativeHandle(),
                                          m_mip_level,
                                          layer);
            }
        }

        // Re-attach depth at specified layer
        if (m_depth_texture && m_desc.has_depth && m_desc.depth_attachment.is_array)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER,
                                      depthAttachmentType(m_desc.depth_attachment.format),
                                      m_depth_texture->getNativeHandle(),
                                      0,
                                      layer);
        }
    }

    void GLFramebuffer::updateColorAttachments()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        for (uint32_t i = 0; i < m_color_textures.size(); ++i)
        {
            if (i >= m_desc.color_attachments.size())
                continue;
            auto& att = m_desc.color_attachments[i];
            if (att.is_array)
            {
                int layer = (m_layer >= 0) ? m_layer : 0;
                glFramebufferTextureLayer(GL_FRAMEBUFFER,
                                          GL_COLOR_ATTACHMENT0 + i,
                                          m_color_textures[i]->getNativeHandle(),
                                          m_mip_level,
                                          layer);
            }
            else
            {
                GLenum target =
                    att.is_cubemap ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + m_cube_face) : GL_TEXTURE_2D;
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0 + i,
                                       target,
                                       m_color_textures[i]->getNativeHandle(),
                                       m_mip_level);
            }
        }
    }

} // namespace RealmEngine
