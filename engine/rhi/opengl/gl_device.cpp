#include "rhi/opengl/gl_device.h"

#include <cstring>
#include <glad/glad.h>
#include <string>

#include "core/log/log_macros.h"
#include "rhi/opengl/gl_buffer.h"
#include "rhi/opengl/gl_framebuffer.h"
#include "rhi/opengl/gl_shader.h"
#include "rhi/opengl/gl_texture.h"
#include "rhi/opengl/gl_vertex_input.h"
#include "rhi/rhi_framebuffer.h"

namespace RealmEngine
{
    // ----- GL debug callback (enabled in Debug builds, requires KHR_debug / GL 4.3+) ---

#if !defined(NDEBUG) && defined(GL_DEBUG_OUTPUT)
    static void GLAPIENTRY glDebugCallback(GLenum                       source,
                                           GLenum                       type,
                                           GLuint                       id,
                                           GLenum                       severity,
                                           [[maybe_unused]] GLsizei     length,
                                           const GLchar*                message,
                                           [[maybe_unused]] const void* user_param)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;
        // Suppress NVIDIA 131204 "texture object (0) does not have a defined base level" -
        // driver warning when unit 0 has default texture; harmless with proper fallback sampling
        if (id == 131204 && message && std::strstr(message, "defined base level"))
            return;
        // Suppress 131170 "driver allocated storage for renderbuffer" - informational only
        if (id == 131169)
            return;
        // Suppress 131218 "vertex shader recompiled based on GL state" - driver quirk;
        // we bind VAO before glUseProgram; some recompilation may still occur with FBO/cascade switches
        if (id == 131218 && message && std::strstr(message, "recompiled"))
            return;


        std::string src_str;
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:
                src_str = "API";
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                src_str = "Window";
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                src_str = "Shader";
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                src_str = "3rdParty";
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                src_str = "App";
                break;
            default:
                src_str = "Other";
                break;
        }

        std::string type_str;
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:
                type_str = "Error";
                break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                type_str = "Deprecated";
                break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                type_str = "UB";
                break;
            case GL_DEBUG_TYPE_PORTABILITY:
                type_str = "Portability";
                break;
            case GL_DEBUG_TYPE_PERFORMANCE:
                type_str = "Performance";
                break;
            default:
                type_str = "Other";
                break;
        }

        std::string msg = "[GL " + src_str + "/" + type_str + " #" + std::to_string(id) + "] " + message;

        if (type == GL_DEBUG_TYPE_ERROR)
            RE_LOG_ERROR(msg);
        else
            RE_LOG_WARN(msg);
    }

    static void enableGLDebugOutput()
    {
        // glDebugMessageCallback requires KHR_debug or GL 4.3+. Check runtime availability.
        if (!glDebugMessageCallback)
        {
            RE_LOG_INFO("GL debug output not available (driver does not expose KHR_debug).");
            return;
        }

        GLint flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            RE_LOG_INFO("GL debug output enabled.");
        }
    }
#endif

    // ----- RHIDevice factory (static) ------------------------------------

    std::unique_ptr<RHIDevice> RHIDevice::create()
    {
        auto device = std::make_unique<GLDevice>();
#if !defined(NDEBUG) && defined(GL_DEBUG_OUTPUT)
        enableGLDebugOutput();
#endif
        return device;
    }

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
        auto shader = std::make_unique<GLShader>(vertex_path, fragment_path);
        if (!shader->isValid())
            return nullptr;
        return shader;
    }

    std::unique_ptr<RHIShader> GLDevice::createShader(const std::string& vertex_path,
                                                      const std::string& geometry_path,
                                                      const std::string& fragment_path)
    {
        auto shader = std::make_unique<GLShader>(vertex_path, geometry_path, fragment_path);
        if (!shader->isValid())
            return nullptr;
        return shader;
    }

    std::unique_ptr<RHIShader> GLDevice::createComputeShader(const std::string& compute_path)
    {
        auto shader = std::make_unique<GLShader>(compute_path, GLShader::ComputeTag{});
        if (!shader->isValid())
            return nullptr;
        return shader;
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

    // ----- Blend ---------------------------------------------------------

    void GLDevice::setBlend(bool enabled)
    {
        if (enabled)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }

    namespace
    {
        GLenum toGLBlendFactor(BlendFactor f)
        {
            switch (f)
            {
                case BlendFactor::Zero:
                    return GL_ZERO;
                case BlendFactor::One:
                    return GL_ONE;
                case BlendFactor::SrcAlpha:
                    return GL_SRC_ALPHA;
                case BlendFactor::OneMinusSrcAlpha:
                    return GL_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::DstAlpha:
                    return GL_DST_ALPHA;
                case BlendFactor::OneMinusDstAlpha:
                    return GL_ONE_MINUS_DST_ALPHA;
                case BlendFactor::SrcColor:
                    return GL_SRC_COLOR;
                case BlendFactor::OneMinusSrcColor:
                    return GL_ONE_MINUS_SRC_COLOR;
                case BlendFactor::DstColor:
                    return GL_DST_COLOR;
                case BlendFactor::OneMinusDstColor:
                    return GL_ONE_MINUS_DST_COLOR;
            }
            return GL_ONE;
        }

        GLenum toGLBlendOp(BlendOp op)
        {
            switch (op)
            {
                case BlendOp::Add:
                    return GL_FUNC_ADD;
                case BlendOp::Subtract:
                    return GL_FUNC_SUBTRACT;
                case BlendOp::ReverseSubtract:
                    return GL_FUNC_REVERSE_SUBTRACT;
                case BlendOp::Min:
                    return GL_MIN;
                case BlendOp::Max:
                    return GL_MAX;
            }
            return GL_FUNC_ADD;
        }

        GLenum toGLStencilOp(StencilOp op)
        {
            switch (op)
            {
                case StencilOp::Keep:
                    return GL_KEEP;
                case StencilOp::Zero:
                    return GL_ZERO;
                case StencilOp::Replace:
                    return GL_REPLACE;
                case StencilOp::Increment:
                    return GL_INCR;
                case StencilOp::IncrementWrap:
                    return GL_INCR_WRAP;
                case StencilOp::Decrement:
                    return GL_DECR;
                case StencilOp::DecrementWrap:
                    return GL_DECR_WRAP;
                case StencilOp::Invert:
                    return GL_INVERT;
            }
            return GL_KEEP;
        }

        GLenum toGLStencilFunc(StencilFunc f)
        {
            switch (f)
            {
                case StencilFunc::Never:
                    return GL_NEVER;
                case StencilFunc::Less:
                    return GL_LESS;
                case StencilFunc::LessEqual:
                    return GL_LEQUAL;
                case StencilFunc::Greater:
                    return GL_GREATER;
                case StencilFunc::GreaterEqual:
                    return GL_GEQUAL;
                case StencilFunc::Equal:
                    return GL_EQUAL;
                case StencilFunc::NotEqual:
                    return GL_NOTEQUAL;
                case StencilFunc::Always:
                    return GL_ALWAYS;
            }
            return GL_ALWAYS;
        }
    } // namespace

    void GLDevice::setBlendFunc(BlendFactor src_rgb, BlendFactor dst_rgb, BlendFactor src_alpha, BlendFactor dst_alpha)
    {
        glBlendFuncSeparate(
            toGLBlendFactor(src_rgb), toGLBlendFactor(dst_rgb), toGLBlendFactor(src_alpha), toGLBlendFactor(dst_alpha));
    }

    void GLDevice::setBlendOp(BlendOp op_rgb, BlendOp op_alpha)
    {
        glBlendEquationSeparate(toGLBlendOp(op_rgb), toGLBlendOp(op_alpha));
    }

    // ----- Stencil -------------------------------------------------------

    void GLDevice::setStencilTest(bool enabled)
    {
        if (enabled)
            glEnable(GL_STENCIL_TEST);
        else
            glDisable(GL_STENCIL_TEST);
    }

    void GLDevice::setStencilFunc(StencilFunc func, int ref, uint32_t mask)
    {
        glStencilFunc(toGLStencilFunc(func), ref, mask);
    }

    void GLDevice::setStencilOp(StencilOp stencil_fail, StencilOp depth_fail, StencilOp depth_pass)
    {
        glStencilOp(toGLStencilOp(stencil_fail), toGLStencilOp(depth_fail), toGLStencilOp(depth_pass));
    }

    void GLDevice::setStencilMask(uint32_t mask) { glStencilMask(mask); }

    // ----- Scissor -------------------------------------------------------

    void GLDevice::setScissorTest(bool enabled)
    {
        if (enabled)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
    }

    void GLDevice::setScissorRect(int x, int y, int w, int h) { glScissor(x, y, w, h); }

    // ----- Polygon mode --------------------------------------------------

    void GLDevice::setPolygonMode(PolygonMode mode)
    {
        switch (mode)
        {
            case PolygonMode::Fill:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case PolygonMode::Line:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case PolygonMode::Point:
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
        }
    }

    // ----- Framebuffer ---------------------------------------------------

    void GLDevice::bindDefaultFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void GLDevice::blitFramebuffer(RHIFramebuffer* src,
                                   RHIFramebuffer* dst,
                                   int             srcX0,
                                   int             srcY0,
                                   int             srcX1,
                                   int             srcY1,
                                   int             dstX0,
                                   int             dstY0,
                                   int             dstX1,
                                   int             dstY1,
                                   BlitMask        mask)
    {
        GLuint src_fbo = src ? src->getNativeHandle() : 0;
        GLuint dst_fbo = dst ? dst->getNativeHandle() : 0;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);

        GLbitfield gl_mask = 0;
        if (mask & BlitMask::Color)
        {
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            gl_mask |= GL_COLOR_BUFFER_BIT;
        }
        if (mask & BlitMask::Depth)
            gl_mask |= GL_DEPTH_BUFFER_BIT;
        if (mask & BlitMask::Stencil)
            gl_mask |= GL_STENCIL_BUFFER_BIT;

        // GL spec: depth/stencil blit requires GL_NEAREST
        GLenum filter = (mask & BlitMask::Depth) || (mask & BlitMask::Stencil) ? GL_NEAREST : GL_LINEAR;
        glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, gl_mask, filter);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ----- Texture helpers -----------------------------------------------

    void GLDevice::bindTexture(uint32_t unit, RHITexture& texture) { texture.bind(unit); }

    // ----- Compute -------------------------------------------------------

    void GLDevice::dispatchCompute(uint32_t groups_x, uint32_t groups_y, uint32_t groups_z)
    {
        glDispatchCompute(groups_x, groups_y, groups_z);
    }

    void GLDevice::memoryBarrier(BarrierFlags flags)
    {
        GLbitfield bits = 0;
        if (flags & BarrierFlags::ShaderStorage)
            bits |= GL_SHADER_STORAGE_BARRIER_BIT;
        if (flags & BarrierFlags::ImageAccess)
            bits |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        if (flags & BarrierFlags::TextureFetch)
            bits |= GL_TEXTURE_FETCH_BARRIER_BIT;
        if (flags & BarrierFlags::BufferUpdate)
            bits |= GL_BUFFER_UPDATE_BARRIER_BIT;
        if (flags & BarrierFlags::Framebuffer)
            bits |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (flags == BarrierFlags::All)
            bits = GL_ALL_BARRIER_BITS;
        glMemoryBarrier(bits);
    }

    // ----- Misc ----------------------------------------------------------

    void GLDevice::enableSeamlessCubemap() { glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }

    void GLDevice::enableMultisample(bool enabled)
    {
        if (enabled)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);
    }

    // ----- GPU info ------------------------------------------------------

    std::string GLDevice::getGPUVendor() const
    {
        const char* v = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        return v ? std::string(v) : "Unknown";
    }

    std::string GLDevice::getGPURenderer() const
    {
        const char* r = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        return r ? std::string(r) : "Unknown";
    }

    std::string GLDevice::getAPIVersion() const
    {
        const char* v = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        return v ? std::string(v) : "Unknown";
    }

    std::string GLDevice::getShadingLanguageVersion() const
    {
        const char* v = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        return v ? std::string(v) : "Unknown";
    }

} // namespace RealmEngine
