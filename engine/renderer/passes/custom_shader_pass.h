#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "renderer/shader_cache.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIBuffer;
    class RHITexture;
    class RHIShader;
    class SceneColorSource;
    class ShadowPass;

    // Forward pass that renders meshes using per-material custom shaders.
    // Works in both Forward and Deferred pipelines by rendering into the
    // scene color source's framebuffer (same depth buffer).
    class CustomShaderPass final : public RenderPass
    {
    public:
        CustomShaderPass();
        ~CustomShaderPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setShadowPass(ShadowPass* sp) { m_shadow_pass = sp; }
        void setIBLTextures(RHITexture* diffuse, RHITexture* prefiltered, RHITexture* brdf);

    private:
        void setupEngineUniforms(RHIShader& shader, const RenderContext& ctx);

        ShaderCache m_shader_cache;

        std::unique_ptr<RHIBuffer> m_light_ubo;

        SceneColorSource* m_scene_color {nullptr};
        ShadowPass*       m_shadow_pass {nullptr};
        RHITexture*       m_ibl_diffuse {nullptr};
        RHITexture*       m_ibl_prefiltered {nullptr};
        RHITexture*       m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
