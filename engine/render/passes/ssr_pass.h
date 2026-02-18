#pragma once

#include <memory>
#include <string>

#include "render/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class SceneColorSource;
    class GBufferPass;
    class HiZPass;
    class FullscreenQuad;

    class SSRPass final : public RenderPass
    {
    public:
        SSRPass(const std::string& shader_path, bool enabled, int max_steps = 64, float max_distance = 100.0f);
        ~SSRPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setHiZPass(HiZPass* pass) { m_hiz_pass = pass; }
        void setGBufferPass(GBufferPass* pass) { m_gbuffer_pass = pass; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }

        bool            isEnabled() const { return m_enabled; }
        RHITexture*     getResultTexture() const;
        RHIFramebuffer* getFramebuffer() const { return m_framebuffer.get(); }

    private:
        std::string m_shader_path;
        bool        m_enabled;
        int         m_max_steps;
        float       m_max_distance;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;

        SceneColorSource* m_scene_color {nullptr};
        HiZPass*          m_hiz_pass {nullptr};
        GBufferPass*      m_gbuffer_pass {nullptr};
        FullscreenQuad*   m_quad {nullptr};
    };

} // namespace RealmEngine
