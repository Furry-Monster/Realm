#pragma once

#include <memory>
#include <string>

#include "functional/render/render_pass.h"
#include "functional/render/rhi/rhi_types.h"
#include "functional/render/scene_color_source.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHIBuffer;
    class RHITexture;
    class GBufferPass;
    class CSMShadowPass;
    class ClusteredLightCullPass;
    class FullscreenQuad;

    // Deferred lighting pass: reads G-Buffer, computes PBR lighting with IBL,
    // writes HDR result to a single RGBA16F + Depth framebuffer that is
    // compatible with all downstream passes (GTAO, Bloom, PostProcess, etc.).
    class DeferredLightingPass final
        : public RenderPass
        , public SceneColorSource
    {
    public:
        explicit DeferredLightingPass(const std::string& shader_path);
        ~DeferredLightingPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        // SceneColorSource
        RHIFramebuffer* getFramebuffer() const override { return m_framebuffer.get(); }

        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }
        void setGBufferPass(GBufferPass* gp) { m_gbuffer_pass = gp; }
        void setShadowPass(CSMShadowPass* sp) { m_shadow_pass = sp; }
        void setClusteredLightCullPass(ClusteredLightCullPass* cp) { m_cluster_cull_pass = cp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setIBLTextures(RHITexture* diffuse_irradiance, RHITexture* prefiltered_env, RHITexture* brdf_lut);

    private:
        std::string m_shader_path;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHIBuffer>      m_light_ssbo;
        std::unique_ptr<RHIBuffer>      m_probe_ssbo;

        GBufferPass*            m_gbuffer_pass {nullptr};
        CSMShadowPass*          m_shadow_pass {nullptr};
        ClusteredLightCullPass* m_cluster_cull_pass {nullptr};
        FullscreenQuad*         m_quad {nullptr};

        RHITexture* m_ibl_diffuse {nullptr};
        RHITexture* m_ibl_prefiltered {nullptr};
        RHITexture* m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
