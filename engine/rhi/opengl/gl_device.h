#pragma once

#include "rhi/rhi_device.h"

namespace RealmEngine
{
    class GLDevice final : public RHIDevice
    {
    public:
        GLDevice()           = default;
        ~GLDevice() override = default;

        // Resource creation
        std::unique_ptr<RHIBuffer>
        createBuffer(BufferType type, BufferUsage usage, const void* data, size_t size) override;

        std::unique_ptr<RHITexture> createTexture(const TextureDesc& desc) override;

        std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                const std::string& fragment_path) override;
        std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                const std::string& geometry_path,
                                                const std::string& fragment_path) override;

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

        // Framebuffer
        void bindDefaultFramebuffer() override;

        // Texture helpers
        void bindTexture(uint32_t unit, RHITexture& texture) override;
        void bindCubemap(uint32_t unit, uint32_t native_handle) override;

        // Misc
        void enableSeamlessCubemap() override;
    };

} // namespace RealmEngine
