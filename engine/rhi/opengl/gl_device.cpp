#include "rhi/opengl/gl_device.h"

#include <glad/gl.h>

#include "rhi/opengl/gl_buffer.h"
#include "rhi/opengl/gl_framebuffer.h"
#include "rhi/opengl/gl_shader.h"
#include "rhi/opengl/gl_texture.h"
#include "rhi/opengl/gl_vertex_input.h"

namespace RealmEngine
{
    // ----- RHIDevice factory (static) ------------------------------------

    std::unique_ptr<RHIDevice> RHIDevice::create() { return std::make_unique<GLDevice>(); }

    // ----- Resource creation ---------------------------------------------

    std::unique_ptr<RHIBuffer> GLDevice::createBuffer(BufferType type, BufferUsage usage, const void* data, size_t size)
    {
        return std::make_unique<GLBuffer>(type, usage, data, size);
    }

    std::unique_ptr<RHITexture> GLDevice::createTexture(const TextureDesc& desc)
    {
        return std::make_unique<GLTexture>(desc);
    }

    std::unique_ptr<RHIShader> GLDevice::createShader(const std::string& vertex_path, const std::string& fragment_path)
    {
        return std::make_unique<GLShader>(vertex_path, fragment_path);
    }

    std::unique_ptr<RHIShader> GLDevice::createShader(const std::string& vertex_path,
                                                      const std::string& geometry_path,
                                                      const std::string& fragment_path)
    {
        return std::make_unique<GLShader>(vertex_path, geometry_path, fragment_path);
    }

    std::unique_ptr<RHIFramebuffer> GLDevice::createFramebuffer(const FramebufferDesc& desc)
    {
        return std::make_unique<GLFramebuffer>(desc);
    }

    std::unique_ptr<RHIVertexInput>
    GLDevice::createVertexInput(const VertexLayout& layout, RHIBuffer& vertex_buffer, RHIBuffer* index_buffer)
    {
        return std::make_unique<GLVertexInput>(layout, vertex_buffer, index_buffer);
    }

    // ----- Render state --------------------------------------------------

    void GLDevice::setViewport(int x, int y, int width, int height) { glViewport(x, y, width, height); }

    void GLDevice::setClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

    void GLDevice::clear(ClearFlags flags)
    {
        GLbitfield bits = 0;
        if (flags & ClearFlags::Color)
            bits |= GL_COLOR_BUFFER_BIT;
        if (flags & ClearFlags::Depth)
            bits |= GL_DEPTH_BUFFER_BIT;
        if (flags & ClearFlags::Stencil)
            bits |= GL_STENCIL_BUFFER_BIT;
        glClear(bits);
    }

    void GLDevice::setDepthTest(bool enabled)
    {
        if (enabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void GLDevice::setDepthFunc(DepthFunc func)
    {
        GLenum gl_func = GL_LESS;
        switch (func)
        {
            case DepthFunc::Less:
                gl_func = GL_LESS;
                break;
            case DepthFunc::LessEqual:
                gl_func = GL_LEQUAL;
                break;
            case DepthFunc::Greater:
                gl_func = GL_GREATER;
                break;
            case DepthFunc::GreaterEqual:
                gl_func = GL_GEQUAL;
                break;
            case DepthFunc::Equal:
                gl_func = GL_EQUAL;
                break;
            case DepthFunc::NotEqual:
                gl_func = GL_NOTEQUAL;
                break;
            case DepthFunc::Always:
                gl_func = GL_ALWAYS;
                break;
            case DepthFunc::Never:
                gl_func = GL_NEVER;
                break;
        }
        glDepthFunc(gl_func);
    }

    void GLDevice::setDepthWrite(bool enabled) { glDepthMask(enabled ? GL_TRUE : GL_FALSE); }

    void GLDevice::setCullFace(CullFace face)
    {
        if (face == CullFace::None)
        {
            glDisable(GL_CULL_FACE);
            return;
        }
        glEnable(GL_CULL_FACE);
        switch (face)
        {
            case CullFace::Front:
                glCullFace(GL_FRONT);
                break;
            case CullFace::Back:
                glCullFace(GL_BACK);
                break;
            case CullFace::FrontAndBack:
                glCullFace(GL_FRONT_AND_BACK);
                break;
            default:
                break;
        }
    }

    // ----- Framebuffer ---------------------------------------------------

    void GLDevice::bindDefaultFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    // ----- Texture helpers -----------------------------------------------

    void GLDevice::bindTexture(uint32_t unit, RHITexture& texture) { texture.bind(unit); }

    void GLDevice::bindCubemap(uint32_t unit, uint32_t native_handle)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, native_handle);
    }

    // ----- Misc ----------------------------------------------------------

    void GLDevice::enableSeamlessCubemap() { glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }

} // namespace RealmEngine
