#pragma once

#include <memory>
#include <string>

#include "functional/render/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class FullscreenQuad;
    class SceneColorSource;
    class BloomPass;
    class GTAOBlurPass;
    class SSRPass;

    class PostProcessPass final : public RenderPass
    {
    public:
        PostProcessPass(const std::string& shader_path,
                        bool               tonemapping,
                        float              gamma,
                        bool               ao_enabled   = false,
                        float              ao_power     = 1.2f,
                        float              ao_intensity = 0.6f);
        ~PostProcessPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setBloomPass(BloomPass* bp) { m_bloom_pass = bp; }
        void setGTAOBlurPass(GTAOBlurPass* pass) { m_gtao_blur_pass = pass; }
        void setSSRPass(SSRPass* pass) { m_ssr_pass = pass; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }

    private:
        std::string                m_shader_path;
        bool                       m_tonemapping;
        float                      m_gamma;
        bool                       m_ao_enabled;
        float                      m_ao_power;
        float                      m_ao_intensity;
        std::unique_ptr<RHIShader> m_shader;
        SceneColorSource*          m_scene_color {nullptr};
        BloomPass*                 m_bloom_pass {nullptr};
        GTAOBlurPass*              m_gtao_blur_pass {nullptr};
        SSRPass*                   m_ssr_pass {nullptr};
        FullscreenQuad*            m_quad {nullptr};
    };

} // namespace RealmEngine
