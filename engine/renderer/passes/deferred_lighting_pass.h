#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHIBuffer;
    class RHITexture;
    class GBufferPass;
    class CSMShadowPass;
    class FullscreenQuad;

    // Deferred lighting pass: reads G-Buffer, computes PBR lighting with IBL,
    // writes HDR result to a single RGBA16F + Depth framebuffer that is
    // compatible with all downstream passes (SSAO, Bloom, PostProcess, etc.).
    class DeferredLightingPass final
        : public RenderPass
        , public SceneColorSource
    {
    public:
        explicit DeferredLightingPass(const std::string& shader_path);
        ~DeferredLightingPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        // SceneColorSource
        RHIFramebuffer* getFramebuffer() const override { return m_framebuffer.get(); }

        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }
        void setGBufferPass(GBufferPass* gp) { m_gbuffer_pass = gp; }
        void setShadowPass(CSMShadowPass* sp) { m_shadow_pass = sp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setIBLTextures(RHITexture* diffuse_irradiance, RHITexture* prefiltered_env, RHITexture* brdf_lut);

    private:
        std::string m_shader_path;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHIBuffer>      m_light_ubo;

        GBufferPass*    m_gbuffer_pass {nullptr};
        CSMShadowPass*  m_shadow_pass {nullptr};
        FullscreenQuad* m_quad {nullptr};

        RHITexture* m_ibl_diffuse {nullptr};
        RHITexture* m_ibl_prefiltered {nullptr};
        RHITexture* m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
