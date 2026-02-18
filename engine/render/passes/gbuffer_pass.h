#pragma once

#include <memory>
#include <string>

#include "render/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;

    // Deferred G-Buffer pass: writes geometry data into MRT.
    //   RT0 (RGBA16F): albedo.rgb, shadingModelID (0=PBR, 1=Subsurface, 2+=reserved)
    //   RT1 (RGBA16F): worldNormal.xyz, metallic
    //   RT2 (RGBA16F): emissive.rgb, roughness
    //   Depth: Depth24Stencil8
    class GBufferPass final : public RenderPass
    {
    public:
        GBufferPass(const std::string& shader_path, float clear_r, float clear_g, float clear_b, float clear_a);
        ~GBufferPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void            setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }
        RHIFramebuffer* getFramebuffer() const { return m_framebuffer.get(); }

    private:
        std::string m_shader_path;
        float       m_clear_r, m_clear_g, m_clear_b, m_clear_a;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHITexture>     m_default_white;
    };

} // namespace RealmEngine
