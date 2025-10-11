#include "graphic_res_manager.h"

#include "utils.h"
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace RealmEngine
{
    void GraphicResourceManager::initialize()
    {
        m_buffers.clear();
        m_textures.clear();
        m_shaders.clear();
        m_programs.clear();
        m_vaos.clear();
        m_fbos.clear();
        m_rbos.clear();

        m_texture_cache.clear();
        m_shader_cache.clear();
        m_program_cache.clear();

        info("GraphicResourceManager initialized.");
    }

    void GraphicResourceManager::disposal()
    {
        for (ProgramHandle program : m_programs)
            glDeleteProgram(program);
        m_programs.clear();
        m_program_cache.clear();

        for (ShaderHandle shader : m_shaders)
            glDeleteShader(shader);
        m_shaders.clear();
        m_shader_cache.clear();

        for (FBOHandle fbo : m_fbos)
            glDeleteFramebuffers(1, &fbo);
        m_fbos.clear();

        for (RBOHandle rbo : m_rbos)
            glDeleteRenderbuffers(1, &rbo);
        m_rbos.clear();

        for (TextureHandle texture : m_textures)
            glDeleteTextures(1, &texture);
        m_textures.clear();
        m_texture_cache.clear();

        for (VAOHandle vao : m_vaos)
            glDeleteVertexArrays(1, &vao);
        m_vaos.clear();

        for (BufferHandle buffer : m_buffers)
            glDeleteBuffers(1, &buffer);
        m_buffers.clear();

        info("GraphicResourceManager disposed all resources.");
    }

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
    BufferHandle GraphicResourceManager::createVertexBuffer(GLsizeiptr size, const void* data, GLenum usage)
    {
        return createBuffer(GL_ARRAY_BUFFER, size, data, usage);
    }
    BufferHandle GraphicResourceManager::createIndexBuffer(GLsizeiptr size, const void* data, GLenum usage)
    {
        return createBuffer(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }
    BufferHandle GraphicResourceManager::createUniformBuffer(GLsizeiptr size, const void* data, GLenum usage)
    {
        return createBuffer(GL_UNIFORM_BUFFER, size, data, usage);
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
    TextureHandle GraphicResourceManager::createTextureCubemap(GLsizei width,
                                                               GLsizei height,
                                                               GLenum  internal_format,
                                                               GLenum  format,
                                                               GLenum  type)
    {
        TextureHandle texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        for (size_t i = 0; i < 6; ++i)
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, format, type, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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
    TextureHandle GraphicResourceManager::loadCubemap(const std::vector<std::string>& face_paths)
    {
        if (face_paths.size() != 6)
        {
            err("Cubemap requires exactly 6 face textures, got " + std::to_string(face_paths.size()));
            return 0;
        }

        TextureHandle texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        for (size_t i = 0; i < 6; ++i)
        {
            int            width, height, nr_components;
            unsigned char* data = stbi_load(face_paths[i].c_str(), &width, &height, &nr_components, 0);

            if (data == nullptr)
            {
                err("Cubemap face failed to load at path: " + face_paths[i]);
                glDeleteTextures(1, &texture);
                return 0;
            }

            GLenum format;
            switch (nr_components)
            {
                case 1:
                    format = GL_RED;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
                default:
                    err("Unsupported cubemap face format with " + std::to_string(nr_components) +
                        " components: " + face_paths[i]);
                    stbi_image_free(data);
                    glDeleteTextures(1, &texture);
                    return 0;
            }

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        m_textures.insert(texture);
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

    ShaderHandle GraphicResourceManager::createShader(GLenum shader_type, const std::string& source_path)
    {
        std::string cache_key = source_path + "_" + std::to_string(shader_type);
        auto        it        = m_shader_cache.find(cache_key);
        if (it != m_shader_cache.end())
            return it->second;

        std::ifstream file_stream(source_path);
        if (!file_stream.is_open())
        {
            err("Failed to open shader file: " + source_path);
            return 0;
        }

        std::stringstream content_stream;
        content_stream << file_stream.rdbuf();
        file_stream.close();

        std::string code_str = content_stream.str();
        if (code_str.empty())
        {
            warn("Shader file is empty: " + source_path);
            return 0;
        }

        const char* code_cstr = code_str.c_str();

        ShaderHandle shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &code_cstr, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char info_log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);

            std::string shader_type_str;
            switch (shader_type)
            {
                case GL_VERTEX_SHADER:
                    shader_type_str = "VERTEX";
                    break;
                case GL_FRAGMENT_SHADER:
                    shader_type_str = "FRAGMENT";
                    break;
                case GL_GEOMETRY_SHADER:
                    shader_type_str = "GEOMETRY";
                    break;
                default:
                    shader_type_str = "UNKNOWN";
                    break;
            }

            err("Shader compilation failed (" + shader_type_str + " SHADER): " + source_path + "\n" + info_log);
            glDeleteShader(shader);
            return 0;
        }

        m_shaders.insert(shader);
        m_shader_cache[cache_key] = shader;
        return shader;
    }
    ProgramHandle GraphicResourceManager::createProgram(const std::vector<ShaderHandle>& shaders)
    {
        if (shaders.empty())
        {
            err("Cannot create program with empty shader list");
            return 0;
        }

        ProgramHandle program = glCreateProgram();

        for (const ShaderHandle& shader : shaders)
        {
            if (shader == 0)
            {
                err("Invalid shader handle in shader list");
                glDeleteProgram(program);
                return 0;
            }
            glAttachShader(program, shader);
        }

        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char info_log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, info_log);
            err("Program linking failed:\n" + std::string(info_log));
            glDeleteProgram(program);
            return 0;
        }

        // detach all shader from program after compiled is recommended.
        for (const ShaderHandle& shader : shaders)
            glDetachShader(program, shader);

        m_programs.insert(program);
        return program;
    }
    ProgramHandle GraphicResourceManager::loadShaderProgram(const std::string& vertex_path,
                                                            const std::string& fragment_path)
    {
        ShaderHandle vertex_shader   = createShader(GL_VERTEX_SHADER, vertex_path);
        ShaderHandle fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_path);

        if (vertex_shader == 0 || fragment_shader == 0)
        {
            if (vertex_shader != 0)
                deleteShader(vertex_shader);
            if (fragment_shader != 0)
                deleteShader(fragment_shader);
            return 0;
        }

        std::vector<ShaderHandle> shaders = {vertex_shader, fragment_shader};
        ProgramHandle             program = createProgram(shaders);

        deleteShader(vertex_shader);
        deleteShader(fragment_shader);

        return program;
    }

    ProgramHandle GraphicResourceManager::loadShaderProgram(const std::string& vertex_path,
                                                            const std::string& fragment_path,
                                                            const std::string& geometry_path)
    {
        ShaderHandle vertex_shader   = createShader(GL_VERTEX_SHADER, vertex_path);
        ShaderHandle fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_path);
        ShaderHandle geometry_shader = createShader(GL_GEOMETRY_SHADER, geometry_path);

        if (vertex_shader == 0 || fragment_shader == 0 || geometry_shader == 0)
        {
            if (vertex_shader != 0)
                deleteShader(vertex_shader);
            if (fragment_shader != 0)
                deleteShader(fragment_shader);
            if (geometry_shader != 0)
                deleteShader(geometry_shader);
            return 0;
        }

        std::vector<ShaderHandle> shaders = {vertex_shader, fragment_shader, geometry_shader};
        ProgramHandle             program = createProgram(shaders);

        deleteShader(vertex_shader);
        deleteShader(fragment_shader);
        deleteShader(geometry_shader);

        return program;
    }

    ProgramHandle GraphicResourceManager::loadShaderProgram(const std::string& name,
                                                            const std::string& vertex_path,
                                                            const std::string& fragment_path,
                                                            const std::string& geometry_path)
    {
        auto it = m_program_cache.find(name);
        if (it != m_program_cache.end())
            return it->second;

        ProgramHandle program;
        if (geometry_path.empty())
            program = loadShaderProgram(vertex_path, fragment_path);
        else
            program = loadShaderProgram(vertex_path, fragment_path, geometry_path);

        if (program != 0)
            m_program_cache[name] = program;

        return program;
    }
    ProgramHandle GraphicResourceManager::getProgram(const std::string& name)
    {
        auto it = m_program_cache.find(name);
        if (it != m_program_cache.end())
            return it->second;

        warn("Invalid program name :" + name);
        return 0;
    }
    void GraphicResourceManager::deleteShader(ShaderHandle handle)
    {
        if (m_shaders.find(handle) == m_shaders.end())
            return;

        glDeleteShader(handle);
        m_shaders.erase(handle);

        for (auto it = m_shader_cache.begin(); it != m_shader_cache.end();)
        {
            if (it->second == handle)
                it = m_shader_cache.erase(it);
            else
                ++it;
        }
    }
    void GraphicResourceManager::deleteProgram(ProgramHandle handle)
    {
        if (m_programs.find(handle) == m_programs.end())
            return;

        glDeleteProgram(handle);
        m_programs.erase(handle);

        for (auto it = m_program_cache.begin(); it != m_program_cache.end();)
        {
            if (it->second == handle)
                it = m_program_cache.erase(it);
            else
                ++it;
        }
    }

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

    FBOHandle GraphicResourceManager::createFramebuffer()
    {
        FBOHandle fbo;
        glGenFramebuffers(1, &fbo);
        m_fbos.insert(fbo);
        return fbo;
    }
    void GraphicResourceManager::attachTextureToFramebuffer(FBOHandle     fbo,
                                                            TextureHandle texture,
                                                            GLenum        attachment,
                                                            GLenum        target)
    {
        if (m_fbos.find(fbo) == m_fbos.end())
        {
            err("Invalid framebuffer handle");
            return;
        }

        if (m_textures.find(texture) == m_textures.end())
        {
            err("Invalid texture handle");
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void
    GraphicResourceManager::attachRenderbufferToFramebuffer(FBOHandle fbo, RBOHandle renderbuffer, GLenum attachment)
    {
        if (m_fbos.find(fbo) == m_fbos.end())
        {
            err("Invalid framebuffer handle");
            return;
        }

        if (m_rbos.find(renderbuffer) == m_rbos.end())
        {
            err("Invalid renderbuffer handle");
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    bool GraphicResourceManager::checkFramebufferComplete(FBOHandle fbo)
    {
        if (m_fbos.find(fbo) == m_fbos.end())
        {
            err("Invalid framebuffer handle");
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            std::string error_msg;
            switch (status)
            {
                case GL_FRAMEBUFFER_UNDEFINED:
                    error_msg = "Framebuffer undefined";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    error_msg = "Framebuffer incomplete attachment";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    error_msg = "Framebuffer incomplete: missing attachment";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    error_msg = "Framebuffer incomplete: draw buffer";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    error_msg = "Framebuffer incomplete: read buffer";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    error_msg = "Framebuffer unsupported";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    error_msg = "Framebuffer incomplete: multisample";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    error_msg = "Framebuffer incomplete: layer targets";
                    break;
                default:
                    error_msg = "Unknown framebuffer error";
                    break;
            }
            err("Framebuffer completeness check failed: " + error_msg);
            return false;
        }

        return true;
    }
    void GraphicResourceManager::deleteFramebuffer(FBOHandle handle)
    {
        if (m_fbos.find(handle) == m_fbos.end())
            return;

        glDeleteFramebuffers(1, &handle);
        m_fbos.erase(handle);
    }

    RBOHandle GraphicResourceManager::createRenderbuffer(GLenum internal_format, GLsizei width, GLsizei height)
    {
        RBOHandle rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        m_rbos.insert(rbo);
        return rbo;
    }

    RBOHandle GraphicResourceManager::createRenderbufferMultisample(GLenum  internal_format,
                                                                    GLsizei samples,
                                                                    GLsizei width,
                                                                    GLsizei height)
    {
        GLint max_samples;
        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
        if (samples > max_samples)
        {
            warn("Requested samples (" + std::to_string(samples) + ") exceeds max supported (" +
                 std::to_string(max_samples) + "), clamping to max");
            samples = max_samples;
        }

        RBOHandle rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internal_format, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        m_rbos.insert(rbo);
        return rbo;
    }

    void GraphicResourceManager::deleteRenderbuffer(RBOHandle handle)
    {
        if (m_rbos.find(handle) == m_rbos.end())
            return;

        glDeleteRenderbuffers(1, &handle);
        m_rbos.erase(handle);
    }

} // namespace RealmEngine