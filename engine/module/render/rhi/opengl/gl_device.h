#pragma once

#include "module/render/rhi/rhi_device.h"

namespace RealmEngine
{
    class GLDevice final : public RHIDevice
    {
    public:
        GLDevice()                    = default;
        ~GLDevice() noexcept override = default;

        // Resource creation
        std::unique_ptr<RHIBuffer>
        createBuffer(BufferType type, BufferUsage usage, const void* data, size_t size) override;

        std::unique_ptr<RHITexture> createTexture(const TextureDesc& desc) override;

        std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                const std::string& fragment_path) override;
        std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                const std::string& geometry_path,
                                                const std::string& fragment_path) override;

        std::unique_ptr<RHIShader> createComputeShader(const std::string& compute_path) override;

        std::unique_ptr<RHIFramebuffer> createFramebuffer(const FramebufferDesc& desc) override;

        std::unique_ptr<RHIVertexInput> createVertexInput(const VertexLayout& layout,
                                                          RHIBuffer&          vertex_buffer,
                                                          RHIBuffer*          index_buffer = nullptr) override;

        // Render state
        void setViewport(int x, int y, int width, int height) override;
        void setClearColor(float r, float g, float b, float a) override;
        void clear(ClearFlags flags) override;
        void setDepthTest(bool enabled) override;
        void setDepthFunc(DepthFunc func) override;
        void setDepthWrite(bool enabled) override;
        void setCullFace(CullFace face) override;

        // Blend / Stencil / Scissor / Polygon
        void setBlend(bool enabled) override;
        void
        setBlendFunc(BlendFactor src_rgb, BlendFactor dst_rgb, BlendFactor src_alpha, BlendFactor dst_alpha) override;
        void setBlendOp(BlendOp op_rgb, BlendOp op_alpha) override;

        void setStencilTest(bool enabled) override;
        void setStencilFunc(StencilFunc func, int ref, uint32_t mask) override;
        void setStencilOp(StencilOp stencil_fail, StencilOp depth_fail, StencilOp depth_pass) override;
        void setStencilMask(uint32_t mask) override;

        void setScissorTest(bool enabled) override;
        void setScissorRect(int x, int y, int w, int h) override;

        void setPolygonMode(PolygonMode mode) override;

        // Framebuffer
        void bindDefaultFramebuffer() override;
        void blitFramebuffer(RHIFramebuffer* src,
                             RHIFramebuffer* dst,
                             int             srcX0,
                             int             srcY0,
                             int             srcX1,
                             int             srcY1,
                             int             dstX0,
                             int             dstY0,
                             int             dstX1,
                             int             dstY1,
                             BlitMask        mask = BlitMask::Color) override;

        // Texture helpers
        void bindTexture(uint32_t unit, RHITexture& texture) override;

        // Compute
        void dispatchCompute(uint32_t groups_x, uint32_t groups_y, uint32_t groups_z) override;
        void memoryBarrier(BarrierFlags flags) override;

        // Misc
        void enableSeamlessCubemap() override;
        void enableMultisample(bool enabled) override;

        // GPU info
        std::string getGPUVendor() const override;
        std::string getGPURenderer() const override;
        std::string getAPIVersion() const override;
        std::string getShaderLangVer() const override;
    };

} // namespace RealmEngine
