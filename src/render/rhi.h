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
    using RBOHandle     = GLuint;

    /**
     * @brief OpenGL Rendering hardware interface.
     *
     */
    class RHI
    {
    public:
        RHI()           = default;
        ~RHI() noexcept = default;

        RHI(const RHI& that)            = delete;
        RHI(RHI&& that)                 = delete;
        RHI& operator=(const RHI& that) = delete;
        RHI& operator=(RHI&& that)      = delete;

        void initialize();
        void disposal();

        // Buffer
        BufferHandle createBuffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
        BufferHandle createVertexBuffer(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        BufferHandle createIndexBuffer(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        BufferHandle createUniformBuffer(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW);
        void updateBuffer(BufferHandle handle, GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
        void deleteBuffer(BufferHandle handle);

        // Texture
        TextureHandle createTexture2D(GLsizei     width,
                                      GLsizei     height,
                                      GLenum      internal_format,
                                      GLenum      format,
                                      GLenum      type,
                                      const void* data = nullptr);
        TextureHandle
        createTextureCubemap(GLsizei width, GLsizei height, GLenum internal_format, GLenum format, GLenum type);
        TextureHandle loadTexture(const std::string& filepath);
        TextureHandle loadCubemap(const std::vector<std::string>& face_paths);
        void          deleteTexture(TextureHandle handle);

        // Shader & Program
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

        // Array Object
        VAOHandle createVAO();
        void      deleteVAO(VAOHandle handle);

        // Framebuffer Object
        FBOHandle createFramebuffer();
        void      attachTextureToFramebuffer(FBOHandle     fbo,
                                             TextureHandle texture,
                                             GLenum        attachment,
                                             GLenum        target = GL_TEXTURE_2D);
        void      attachRenderbufferToFramebuffer(FBOHandle fbo, RBOHandle renderbuffer, GLenum attachment);
        bool      checkFramebufferComplete(FBOHandle fbo);
        void      deleteFramebuffer(FBOHandle handle);

        // Renderbuffer Object
        RBOHandle createRenderbuffer(GLenum internal_format, GLsizei width, GLsizei height);
        RBOHandle createRenderbufferMultisample(GLenum internal_format, GLsizei samples, GLsizei width, GLsizei height);
        void      deleteRenderbuffer(RBOHandle handle);

    private:
        std::unordered_set<BufferHandle>  m_buffers;
        std::unordered_set<TextureHandle> m_textures;
        std::unordered_set<ShaderHandle>  m_shaders;
        std::unordered_set<ProgramHandle> m_programs;
        std::unordered_set<VAOHandle>     m_vaos;
        std::unordered_set<FBOHandle>     m_fbos;
        std::unordered_set<RBOHandle>     m_rbos;

        std::unordered_map<std::string, TextureHandle> m_texture_cache;
        std::unordered_map<std::string, ShaderHandle>  m_shader_cache;
        std::unordered_map<std::string, ProgramHandle> m_program_cache;
    };

} // namespace RealmEngine