#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class GTAOPass;
    class SceneColorSource;
    class FullscreenQuad;

    class GTAOBlurPass final : public RenderPass
    {
    public:
        explicit GTAOBlurPass(const std::string& shader_path);
        ~GTAOBlurPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setGTAOPass(GTAOPass* pass) { m_gtao_pass = pass; }
        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }

        RHITexture* getResultTexture() const;

    private:
        std::string                     m_shader_path;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        GTAOPass*                       m_gtao_pass {nullptr};
        SceneColorSource*               m_scene_color {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
    };

} // namespace RealmEngine
