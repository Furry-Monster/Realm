#pragma once

#include <memory>
#include <string>

#include "functional/render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIBuffer;
    class RHITexture;
    class RHIShader;
    class RHIFramebuffer;
    class RHIVertexInput;

    // Abstract graphics device -- factory for GPU resources + render-state management.
    // One concrete implementation per backend (OpenGL, Vulkan, D3D12, ...).
    class RHIDevice
    {
    public:
        virtual ~RHIDevice() noexcept = default;

        // ----- Resource creation ------------------------------------------

        virtual std::unique_ptr<RHIBuffer>
        createBuffer(BufferType type, BufferUsage usage, const void* data, size_t size) = 0;

        virtual std::unique_ptr<RHITexture> createTexture(const TextureDesc& desc) = 0;

        virtual std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                        const std::string& fragment_path) = 0;
        virtual std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                        const std::string& geometry_path,
                                                        const std::string& fragment_path) = 0;

        virtual std::unique_ptr<RHIShader> createComputeShader(const std::string& compute_path) = 0;

        virtual std::unique_ptr<RHIFramebuffer> createFramebuffer(const FramebufferDesc& desc) = 0;

        virtual std::unique_ptr<RHIVertexInput>
        createVertexInput(const VertexLayout& layout, RHIBuffer& vertex_buffer, RHIBuffer* index_buffer = nullptr) = 0;

        // ----- Render state -----------------------------------------------

        virtual void setViewport(int x, int y, int width, int height)  = 0;
        virtual void setClearColor(float r, float g, float b, float a) = 0;
        virtual void clear(ClearFlags flags)                           = 0;

        virtual void setDepthTest(bool enabled)   = 0;
        virtual void setDepthFunc(DepthFunc func) = 0;
        virtual void setDepthWrite(bool enabled)  = 0;

        virtual void setCullFace(CullFace face) = 0;

        // ----- Blend / Stencil / Scissor / Polygon -----------------------

        virtual void setBlend(bool enabled) = 0;
        virtual void
        setBlendFunc(BlendFactor src_rgb, BlendFactor dst_rgb, BlendFactor src_alpha, BlendFactor dst_alpha) = 0;
        virtual void setBlendOp(BlendOp op_rgb, BlendOp op_alpha)                                            = 0;

        virtual void setStencilTest(bool enabled)                                                     = 0;
        virtual void setStencilFunc(StencilFunc func, int ref, uint32_t mask)                         = 0;
        virtual void setStencilOp(StencilOp stencil_fail, StencilOp depth_fail, StencilOp depth_pass) = 0;
        virtual void setStencilMask(uint32_t mask)                                                    = 0;

        virtual void setScissorTest(bool enabled)               = 0;
        virtual void setScissorRect(int x, int y, int w, int h) = 0;

        virtual void setPolygonMode(PolygonMode mode) = 0;

        // ----- Framebuffer binding ----------------------------------------

        virtual void bindDefaultFramebuffer() = 0;

        virtual void blitFramebuffer(RHIFramebuffer* src,
                                     RHIFramebuffer* dst,
                                     int             srcX0,
                                     int             srcY0,
                                     int             srcX1,
                                     int             srcY1,
                                     int             dstX0,
                                     int             dstY0,
                                     int             dstX1,
                                     int             dstY1,
                                     BlitMask        mask = BlitMask::Color) = 0;

        // ----- Texture helpers (bind texture to unit) ---------------------

        virtual void bindTexture(uint32_t unit, RHITexture& texture) = 0;

        // ----- Compute dispatch -----------------------------------------------

        virtual void dispatchCompute(uint32_t groups_x, uint32_t groups_y, uint32_t groups_z) = 0;
        virtual void memoryBarrier(BarrierFlags flags)                                        = 0;

        // ----- Misc -------------------------------------------------------

        virtual void enableSeamlessCubemap()         = 0;
        virtual void enableMultisample(bool enabled) = 0;

        // ----- GPU info (for platform diagnostics) ------------------------

        virtual std::string getGPUVendor() const     = 0;
        virtual std::string getGPURenderer() const   = 0;
        virtual std::string getAPIVersion() const    = 0;
        virtual std::string getShaderLangVer() const = 0;

        // ----- Factory ----------------------------------------------------

        static std::unique_ptr<RHIDevice> create(); // returns platform default (OpenGL for now)
    };

} // namespace RealmEngine
