#pragma once

#include <glad/glad.h>

#include "module/render/rhi/rhi_types.h"

namespace RealmEngine
{
    // -----Utilities-------------------------
    // Shared OpenGL format conversion utilities.
    // Used by both GLTexture and GLFramebuffer.

    inline GLenum toGLInternalFormat(TextureFormat fmt)
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
            case TextureFormat::SRGB8:
                return GL_SRGB8;
            case TextureFormat::SRGBA8:
                return GL_SRGB8_ALPHA8;
            case TextureFormat::R16F:
                return GL_R16F;
            case TextureFormat::RG16F:
                return GL_RG16F;
            case TextureFormat::RGB16F:
                return GL_RGB16F;
            case TextureFormat::RGBA16F:
                return GL_RGBA16F;
            case TextureFormat::R32F:
                return GL_R32F;
            case TextureFormat::RG32F:
                return GL_RG32F;
            case TextureFormat::RGBA32F:
                return GL_RGBA32F;
            case TextureFormat::R32UI:
                return GL_R32UI;
            case TextureFormat::RG16UI:
                return GL_RG16UI;
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

    inline GLenum toGLPixelFormat(TextureFormat fmt)
    {
        switch (fmt)
        {
            case TextureFormat::R8:
            case TextureFormat::R16F:
            case TextureFormat::R32F:
                return GL_RED;
            case TextureFormat::R32UI:
                return GL_RED_INTEGER;
            case TextureFormat::RG8:
            case TextureFormat::RG16F:
            case TextureFormat::RG32F:
                return GL_RG;
            case TextureFormat::RG16UI:
                return GL_RG_INTEGER;
            case TextureFormat::RGB8:
            case TextureFormat::SRGB8:
            case TextureFormat::RGB16F:
                return GL_RGB;
            case TextureFormat::RGBA8:
            case TextureFormat::SRGBA8:
            case TextureFormat::RGBA16F:
            case TextureFormat::RGBA32F:
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

    inline GLenum toGLPixelType(TextureFormat fmt)
    {
        switch (fmt)
        {
            case TextureFormat::R16F:
            case TextureFormat::RG16F:
            case TextureFormat::RGB16F:
            case TextureFormat::RGBA16F:
            case TextureFormat::R32F:
            case TextureFormat::RG32F:
            case TextureFormat::RGBA32F:
            case TextureFormat::Depth32F:
                return GL_FLOAT;
            case TextureFormat::R32UI:
                return GL_UNSIGNED_INT;
            case TextureFormat::RG16UI:
            case TextureFormat::Depth16:
                return GL_UNSIGNED_SHORT;
            case TextureFormat::Depth24:
                return GL_UNSIGNED_INT;
            case TextureFormat::Depth24Stencil8:
                return GL_UNSIGNED_INT_24_8;
            default:
                return GL_UNSIGNED_BYTE;
        }
    }

    inline GLint toGLFilter(TextureFilter f)
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

    inline GLint toGLWrap(TextureWrap w)
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

    inline GLenum toGLTarget(TextureType type)
    {
        switch (type)
        {
            case TextureType::TextureCube:
                return GL_TEXTURE_CUBE_MAP;
            case TextureType::Texture2DArray:
                return GL_TEXTURE_2D_ARRAY;
            case TextureType::Texture3D:
                return GL_TEXTURE_3D;
            default:
                return GL_TEXTURE_2D;
        }
    }

    inline GLenum toGLAccess(TextureAccess access)
    {
        switch (access)
        {
            case TextureAccess::ReadOnly:
                return GL_READ_ONLY;
            case TextureAccess::WriteOnly:
                return GL_WRITE_ONLY;
            case TextureAccess::ReadWrite:
                return GL_READ_WRITE;
        }
        return GL_READ_WRITE;
    }

} // namespace RealmEngine
