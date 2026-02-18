#pragma once

#include <memory>
#include <string>

#include "render/render_pass.h"
#include "render/rhi/rhi_types.h"
#include "render/shader_cache.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIBuffer;
    class RHITexture;
    class SceneColorSource;
    class CSMShadowPass;
    class RenderMesh;
    class Material;

    class TransparentPass final : public RenderPass
    {
    public:
        explicit TransparentPass(const std::string& shader_path);
        ~TransparentPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setShadowPass(CSMShadowPass* sp) { m_shadow_pass = sp; }
        void setIBLTextures(RHITexture* diffuse, RHITexture* prefiltered, RHITexture* brdf);

        void reloadShaders() { m_shader_cache.clear(); }

    private:
        RHIShader* resolveShader(const Material& mat, RHIDevice& device);
        void       setupEngineUniforms(RHIShader& shader, const RenderContext& ctx);

        std::string m_shader_path;

        std::unique_ptr<RHIShader>  m_pbr_shader;
        std::unique_ptr<RHIBuffer>  m_light_ssbo;
        std::unique_ptr<RHIBuffer>  m_probe_ssbo;
        std::unique_ptr<RHITexture> m_default_white;

        ShaderCache m_shader_cache;

        SceneColorSource* m_scene_color {nullptr};
        CSMShadowPass*    m_shadow_pass {nullptr};
        RHITexture*       m_ibl_diffuse {nullptr};
        RHITexture*       m_ibl_prefiltered {nullptr};
        RHITexture*       m_ibl_brdf {nullptr};
    };

} // namespace RealmEngine
