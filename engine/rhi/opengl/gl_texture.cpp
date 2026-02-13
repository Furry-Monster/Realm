#include "rhi/opengl/gl_texture.h"

#include <glad/gl.h>

namespace RealmEngine
{
    namespace
    {
        GLenum toGLTarget(TextureType type)
        {
            return (type == TextureType::TextureCube) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        }

        GLenum toGLInternalFormat(TextureFormat fmt)
        {
            switch (fmt)
            {
                case TextureFormat::R8:
                    return GL_R8;
                case TextureFormat::RG8:
                    return GL_RG8;
                case TextureFormat::RGB8:
                    return GL_RGB8;
                case TextureFormat::RGBA8:
                    return GL_RGBA8;
                case TextureFormat::R16F:
                    return GL_R16F;
                case TextureFormat::RG16F:
                    return GL_RG16F;
                case TextureFormat::RGB16F:
                    return GL_RGB16F;
                case TextureFormat::RGBA16F:
                    return GL_RGBA16F;
                case TextureFormat::Depth16:
                    return GL_DEPTH_COMPONENT16;
                case TextureFormat::Depth24:
                    return GL_DEPTH_COMPONENT24;
                case TextureFormat::Depth32F:
                    return GL_DEPTH_COMPONENT32F;
                case TextureFormat::Depth24Stencil8:
                    return GL_DEPTH24_STENCIL8;
            }
            return GL_RGBA8;
        }

        GLenum toGLPixelFormat(TextureFormat fmt)
        {
            switch (fmt)
            {
                case TextureFormat::R8:
                case TextureFormat::R16F:
                    return GL_RED;
                case TextureFormat::RG8:
                case TextureFormat::RG16F:
                    return GL_RG;
                case TextureFormat::RGB8:
                case TextureFormat::RGB16F:
                    return GL_RGB;
                case TextureFormat::RGBA8:
                case TextureFormat::RGBA16F:
                    return GL_RGBA;
                case TextureFormat::Depth16:
                case TextureFormat::Depth24:
                case TextureFormat::Depth32F:
                    return GL_DEPTH_COMPONENT;
                case TextureFormat::Depth24Stencil8:
                    return GL_DEPTH_STENCIL;
            }
            return GL_RGBA;
        }

        GLenum toGLPixelType(TextureFormat fmt)
        {
            switch (fmt)
            {
                case TextureFormat::R16F:
                case TextureFormat::RG16F:
                case TextureFormat::RGB16F:
                case TextureFormat::RGBA16F:
                case TextureFormat::Depth32F:
                    return GL_FLOAT;
                case TextureFormat::Depth24Stencil8:
                    return GL_UNSIGNED_INT_24_8;
                default:
                    return GL_UNSIGNED_BYTE;
            }
        }

        GLint toGLFilter(TextureFilter f)
        {
            switch (f)
            {
                case TextureFilter::Nearest:
                    return GL_NEAREST;
                case TextureFilter::Linear:
                    return GL_LINEAR;
                case TextureFilter::NearestMipmapNearest:
                    return GL_NEAREST_MIPMAP_NEAREST;
                case TextureFilter::LinearMipmapNearest:
                    return GL_LINEAR_MIPMAP_NEAREST;
                case TextureFilter::NearestMipmapLinear:
                    return GL_NEAREST_MIPMAP_LINEAR;
                case TextureFilter::LinearMipmapLinear:
                    return GL_LINEAR_MIPMAP_LINEAR;
            }
            return GL_LINEAR;
        }

        GLint toGLWrap(TextureWrap w)
        {
            switch (w)
            {
                case TextureWrap::Repeat:
                    return GL_REPEAT;
                case TextureWrap::ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
                case TextureWrap::ClampToBorder:
                    return GL_CLAMP_TO_BORDER;
                case TextureWrap::MirroredRepeat:
                    return GL_MIRRORED_REPEAT;
            }
            return GL_CLAMP_TO_EDGE;
        }
    } // namespace

    GLTexture::GLTexture(const TextureDesc& desc)
    {
        m_type   = desc.type;
        m_format = desc.format;
        m_width  = desc.width;
        m_height = desc.height;
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
        else // TextureCube
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

        glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, toGLFilter(desc.min_filter));
        glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, toGLFilter(desc.mag_filter));
        glTexParameteri(m_target, GL_TEXTURE_WRAP_S, toGLWrap(desc.wrap_s));
        glTexParameteri(m_target, GL_TEXTURE_WRAP_T, toGLWrap(desc.wrap_t));
        if (desc.type == TextureType::TextureCube)
            glTexParameteri(m_target, GL_TEXTURE_WRAP_R, toGLWrap(desc.wrap_r));

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

    GLTexture::~GLTexture()
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
        glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, width, height, 0, pixel_fmt, pixel_type, nullptr);
        glBindTexture(m_target, 0);
    }

} // namespace RealmEngine
