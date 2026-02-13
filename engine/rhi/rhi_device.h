#pragma once

#include <memory>
#include <string>

#include "rhi/rhi_types.h"

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
        virtual ~RHIDevice() = default;

        // ----- Resource creation ------------------------------------------

        virtual std::unique_ptr<RHIBuffer>
        createBuffer(BufferType type, BufferUsage usage, const void* data, size_t size) = 0;

        virtual std::unique_ptr<RHITexture> createTexture(const TextureDesc& desc) = 0;

        virtual std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                        const std::string& fragment_path) = 0;
        virtual std::unique_ptr<RHIShader> createShader(const std::string& vertex_path,
                                                        const std::string& geometry_path,
                                                        const std::string& fragment_path) = 0;

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

        // ----- Framebuffer binding ----------------------------------------

        virtual void bindDefaultFramebuffer() = 0;

        // ----- Texture helpers (bind texture to unit) ---------------------

        virtual void bindTexture(uint32_t unit, RHITexture& texture)    = 0;
        virtual void bindCubemap(uint32_t unit, uint32_t native_handle) = 0;

        // ----- Misc -------------------------------------------------------

        virtual void enableSeamlessCubemap() = 0;

        // ----- Factory ----------------------------------------------------

        static std::unique_ptr<RHIDevice> create(); // returns platform default (OpenGL for now)
    };

} // namespace RealmEngine
