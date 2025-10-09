#include "resource_manager.h"

#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace RealmEngine
{
    void GraphicResourceManager::initialize() {}

    void GraphicResourceManager::disposal() {}

    BufferHandle GraphicResourceManager::createBuffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
    {
        BufferHandle buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(target, buffer);
        glBufferData(target, size, data, usage);
        glBindBuffer(target, 0);

        m_buffers.insert(buffer);
        return buffer;
    }
    void GraphicResourceManager::updateBuffer(BufferHandle handle,
                                              GLenum       target,
                                              GLintptr     offset,
                                              GLsizeiptr   size,
                                              const void*  data)
    {
        if (m_buffers.find(handle) == m_buffers.end())
            return;

        glBindBuffer(target, handle);
        glBufferSubData(target, offset, size, data);
        glBindBuffer(target, 0);
    }
    void GraphicResourceManager::deleteBuffer(BufferHandle handle)
    {
        if (m_buffers.find(handle) == m_buffers.end())
            return;

        glDeleteBuffers(1, &handle);
        m_buffers.erase(handle);
    }

    TextureHandle GraphicResourceManager::createTexture2D(GLsizei     width,
                                                          GLsizei     height,
                                                          GLenum      internal_format,
                                                          GLenum      format,
                                                          GLenum      type,
                                                          const void* data)
    {
        TextureHandle texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);

        if (data != nullptr)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        m_textures.insert(texture);
        return texture;
    }
    TextureHandle GraphicResourceManager::loadTexture(const std::string& filepath)
    {
        auto it = m_texture_cache.find(filepath);
        if (it != m_texture_cache.end())
            return it->second;

        int            width, height, nr_components;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nr_components, 0);
        if (data == nullptr)
        {
            err("Texture failed to load at path: " + filepath);
            return 0;
        }

        GLenum format;
        GLenum internal_format;
        switch (nr_components)
        {
            case 1:
                format          = GL_RED;
                internal_format = GL_RED;
                break;
            case 3:
                format          = GL_RGB;
                internal_format = GL_RGB;
                break;
            case 4:
                format          = GL_RGBA;
                internal_format = GL_RGBA;
                break;
            default:
                err("Unsupported texture format with " + std::to_string(nr_components) + " components: " + filepath);
                stbi_image_free(data);
                return 0;
        }

        TextureHandle texture = createTexture2D(width, height, internal_format, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        if (texture != 0)
            m_texture_cache[filepath] = texture;

        return texture;
    }
    void GraphicResourceManager::deleteTexture(TextureHandle handle)
    {
        if (m_textures.find(handle) == m_textures.end())
            return;

        glDeleteTextures(1, &handle);
        m_textures.erase(handle);

        for (auto it = m_texture_cache.begin(); it != m_texture_cache.end();)
        {
            if (it->second == handle)
                it = m_texture_cache.erase(it);
            else
                ++it;
        }
    }

    ShaderHandle GraphicResourceManager::createShader(GLenum shaderType, const std::string& source)
    {
        ShaderHandle shader;
        return shader;
    }
    ProgramHandle GraphicResourceManager::createProgram(const std::vector<ShaderHandle>& shaders)
    {
        ProgramHandle program;
        return program;
    }
    ProgramHandle GraphicResourceManager::loadShaderProgram(const std::string& vertex_path,
                                                            const std::string& fragment_path)
    {}
    ProgramHandle GraphicResourceManager::getProgram(const std::string& name) {}
    void          GraphicResourceManager::deleteShader(ShaderHandle handle) {}
    void          GraphicResourceManager::deleteProgram(ProgramHandle handle) {}

    VAOHandle GraphicResourceManager::createVAO()
    {
        VAOHandle vao;
        glGenVertexArrays(1, &vao);
        m_vaos.insert(vao);
        return vao;
    }
    void GraphicResourceManager::deleteVAO(VAOHandle handle)
    {
        if (m_vaos.find(handle) == m_vaos.end())
            return;
        glDeleteVertexArrays(1, &handle);
        m_vaos.erase(handle);
    }

    FBOHandle GraphicResourceManager::createFramebuffer() {}
    void      GraphicResourceManager::deleteFramebuffer(FBOHandle handle) {}

} // namespace RealmEngine