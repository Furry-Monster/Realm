#pragma once

#include <glad/gl.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RealmEngine
{

    using BufferHandle  = GLuint;
    using TextureHandle = GLuint;
    using ShaderHandle  = GLuint;
    using ProgramHandle = GLuint;
    using VAOHandle     = GLuint;
    using FBOHandle     = GLuint;

    class GraphicResourceManager
    {
    public:
        GraphicResourceManager()           = default;
        ~GraphicResourceManager() noexcept = default;

        GraphicResourceManager(const GraphicResourceManager& that)            = delete;
        GraphicResourceManager(GraphicResourceManager&& that)                 = delete;
        GraphicResourceManager& operator=(const GraphicResourceManager& that) = delete;
        GraphicResourceManager& operator=(GraphicResourceManager&& that)      = delete;

        void initialize();
        void disposal();

        BufferHandle createBuffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
        void updateBuffer(BufferHandle handle, GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
        void deleteBuffer(BufferHandle handle);

        TextureHandle createTexture2D(int         width,
                                      int         height,
                                      GLenum      internal_format,
                                      GLenum      format,
                                      GLenum      type,
                                      const void* data = nullptr);
        TextureHandle loadTexture(const std::string& filepath);
        void          deleteTexture(TextureHandle handle);

        ShaderHandle  createShader(GLenum shader_type, const std::string& source_path);
        ProgramHandle createProgram(const std::vector<ShaderHandle>& shaders);
        ProgramHandle loadShaderProgram(const std::string& vertex_path, const std::string& fragment_path);
        ProgramHandle loadShaderProgram(const std::string& vertex_path,
                                        const std::string& fragment_path,
                                        const std::string& geometry_path);
        ProgramHandle loadShaderProgram(const std::string& name,
                                        const std::string& vertex_path,
                                        const std::string& fragment_path,
                                        const std::string& geometry_path);
        ProgramHandle getProgram(const std::string& name);
        void          deleteShader(ShaderHandle handle);
        void          deleteProgram(ProgramHandle handle);

        VAOHandle createVAO();
        void      deleteVAO(VAOHandle handle);

        FBOHandle createFramebuffer();
        void      deleteFramebuffer(FBOHandle handle);

    private:
        std::unordered_set<BufferHandle>  m_buffers;
        std::unordered_set<TextureHandle> m_textures;
        std::unordered_set<ShaderHandle>  m_shaders;
        std::unordered_set<ProgramHandle> m_programs;
        std::unordered_set<VAOHandle>     m_vaos;
        std::unordered_set<FBOHandle>     m_fbos;

        std::unordered_map<std::string, TextureHandle> m_texture_cache;
        std::unordered_map<std::string, ShaderHandle>  m_shader_cache;
        std::unordered_map<std::string, ProgramHandle> m_program_cache;
    };

} // namespace RealmEngine