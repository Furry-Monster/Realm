#include "resource_manager.h"

#include "utils.h"
#include <fstream>
#include <sstream>
#include <string>

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

    FBOHandle GraphicResourceManager::createFramebuffer() {}
    void      GraphicResourceManager::deleteFramebuffer(FBOHandle handle) {}

} // namespace RealmEngine