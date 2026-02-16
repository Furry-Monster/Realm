#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "renderer/scene_color_source.h"
#include "renderer/shader_cache.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHIBuffer;
    class RHITexture;
    class ShadowPass;
    class RenderMesh;
    class Material;

    class OpaquePass final
        : public RenderPass
        , public SceneColorSource
    {
    public:
        OpaquePass(const std::string& shader_path, float clear_r, float clear_g, float clear_b, float clear_a);
        ~OpaquePass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setShadowPass(ShadowPass* shadow) { m_shadow_pass = shadow; }
        void setIBLTextures(RHITexture* diffuse, RHITexture* prefiltered, RHITexture* brdf);

        void            setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }
        RHIFramebuffer* getFramebuffer() const override { return m_framebuffer.get(); }

        // In deferred mode, only render non-deferred-eligible meshes into the
        // scene color source's framebuffer (which is owned by DeferredLightingPass).
        void setDeferredMode(bool deferred, SceneColorSource* src = nullptr)
        {
            m_deferred_mode = deferred;
            m_scene_color   = src;
        }

        void reloadShaders() { m_shader_cache.clear(); }

    private:
        RHIShader* resolveShader(const Material& mat, RHIDevice& device);
        void       setupEngineUniforms(RHIShader& shader, const RenderContext& ctx);

        std::string m_shader_path;
        float       m_clear_r, m_clear_g, m_clear_b, m_clear_a;

        std::unique_ptr<RHIShader>      m_pbr_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHIBuffer>      m_light_ubo;

        ShaderCache m_shader_cache;

        bool              m_deferred_mode {false};
        SceneColorSource* m_scene_color {nullptr};

        ShadowPass* m_shadow_pass {nullptr};
        RHITexture* m_ibl_diffuse {nullptr};
        RHITexture* m_ibl_prefiltered {nullptr};
        RHITexture* m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
