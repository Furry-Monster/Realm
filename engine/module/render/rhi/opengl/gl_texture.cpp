#include "module/render/rhi/opengl/gl_texture.h"

#include <glad/glad.h>

#include "module/render/rhi/opengl/gl_format_utils.h"

namespace RealmEngine
{

    GLTexture::GLTexture(const TextureDesc& desc)
    {
        m_type   = desc.type;
        m_format = desc.format;
        m_width  = desc.width;
        m_height = desc.height;
        m_depth  = desc.depth;
        m_target = toGLTarget(desc.type);
        m_owned  = true;

        GLenum internal_fmt = toGLInternalFormat(desc.format);
        GLenum pixel_fmt    = toGLPixelFormat(desc.format);
        GLenum pixel_type   = toGLPixelType(desc.format);

        glGenTextures(1, &m_id);
        glBindTexture(m_target, m_id);

        if (desc.type == TextureType::Texture2D)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, m_width, m_height, 0, pixel_fmt, pixel_type, desc.data);
        }
        else if (desc.type == TextureType::TextureCube)
        {
            for (int face = 0; face < 6; ++face)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                             0,
                             internal_fmt,
                             m_width,
                             m_height,
                             0,
                             pixel_fmt,
                             pixel_type,
                             nullptr);
        }
        else if (desc.type == TextureType::Texture2DArray)
        {
            glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, internal_fmt, m_width, m_height, m_depth, 0, pixel_fmt, pixel_type, desc.data);
        }
        else if (desc.type == TextureType::Texture3D)
        {
            glTexImage3D(
                GL_TEXTURE_3D, 0, internal_fmt, m_width, m_height, m_depth, 0, pixel_fmt, pixel_type, desc.data);
        }

        glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, toGLFilter(desc.min_filter));
        glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, toGLFilter(desc.mag_filter));
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, toGLWrap(desc.wrap_s));
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, toGLWrap(desc.wrap_t));
        if (desc.type == TextureType::TextureCube || desc.type == TextureType::Texture2DArray ||
            desc.type == TextureType::Texture3D)
            glTexParameteri(m_target, GL_TEXTURE_WRAP_R, toGLWrap(desc.wrap_r));
        if (!desc.gen_mips)
        {
            glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
        }

        if (desc.gen_mips)
            glGenerateMipmap(m_target);

        glBindTexture(m_target, 0);
    }

    GLTexture::GLTexture(uint32_t native_id, TextureType type, TextureFormat format, int width, int height) :
        m_id(native_id), m_target(toGLTarget(type)), m_owned(false)
    {
        m_type   = type;
        m_format = format;
        m_width  = width;
        m_height = height;
    }

    GLTexture::~GLTexture() noexcept
    {
        if (m_owned && m_id != 0)
            glDeleteTextures(1, &m_id);
    }

    void GLTexture::bind(uint32_t unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(m_target, m_id);
    }

    void GLTexture::unbind(uint32_t unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(m_target, 0);
    }

    void GLTexture::generateMipmaps()
    {
        glBindTexture(m_target, m_id);
        glGenerateMipmap(m_target);
        glBindTexture(m_target, 0);
    }

    void GLTexture::resize(int width, int height)
    {
        m_width  = width;
        m_height = height;

        GLenum internal_fmt = toGLInternalFormat(m_format);
        GLenum pixel_fmt    = toGLPixelFormat(m_format);
        GLenum pixel_type   = toGLPixelType(m_format);

        glBindTexture(m_target, m_id);

        if (m_type == TextureType::TextureCube)
        {
            for (int face = 0; face < 6; ++face)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                             0,
                             internal_fmt,
                             width,
                             height,
                             0,
                             pixel_fmt,
                             pixel_type,
                             nullptr);
        }
        else if (m_type == TextureType::Texture2DArray)
        {
            glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, internal_fmt, width, height, m_depth, 0, pixel_fmt, pixel_type, nullptr);
        }
        else if (m_type == TextureType::Texture3D)
        {
            glTexImage3D(GL_TEXTURE_3D, 0, internal_fmt, width, height, m_depth, 0, pixel_fmt, pixel_type, nullptr);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, width, height, 0, pixel_fmt, pixel_type, nullptr);
        }

        glBindTexture(m_target, 0);
    }

    void GLTexture::bindImage(uint32_t unit, int level, bool layered, TextureAccess access)
    {
        glBindImageTexture(
            unit, m_id, level, layered ? GL_TRUE : GL_FALSE, 0, toGLAccess(access), toGLInternalFormat(m_format));
    }

} // namespace RealmEngine
