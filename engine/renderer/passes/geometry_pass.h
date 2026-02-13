#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHIBuffer;
    class RHITexture;
    class ShadowPass;

    // Main PBR geometry pass: renders all objects into HDR + bloom framebuffers.
    class GeometryPass final : public RenderPass
    {
    public:
        GeometryPass(const std::string& shader_path,
                     float              clear_r,
                     float              clear_g,
                     float              clear_b,
                     float              clear_a,
                     float              bloom_brightness_cutoff);
        ~GeometryPass() override;

        void initialize(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        // External dependencies set by Renderer after construction
        void setShadowPass(ShadowPass* shadow) { m_shadow_pass = shadow; }
        void setIBLTextures(RHITexture* diffuse_irradiance, RHITexture* prefiltered_env, RHITexture* brdf_lut);

        void            setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }
        RHIFramebuffer* getFramebuffer() const { return m_framebuffer.get(); }

    private:
        std::string m_shader_path;
        float       m_clear_r, m_clear_g, m_clear_b, m_clear_a;
        float       m_bloom_brightness_cutoff;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHIBuffer>      m_light_ubo;

        ShadowPass* m_shadow_pass {nullptr};

        // IBL textures (non-owning, managed by Renderer)
        RHITexture* m_ibl_diffuse {nullptr};
        RHITexture* m_ibl_prefiltered {nullptr};
        RHITexture* m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
