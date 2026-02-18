#pragma once

#include <memory>
#include <string>

#include "module/render/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class SceneColorSource;
    class FullscreenQuad;

    class GTAOPass final : public RenderPass
    {
    public:
        GTAOPass(const std::string& shader_path, bool enabled, float radius, int num_directions, int num_steps);
        ~GTAOPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }

        bool            isEnabled() const { return m_enabled; }
        RHITexture*     getResultTexture() const;
        RHIFramebuffer* getFramebuffer() const { return m_framebuffer.get(); }

    private:
        void generateNoiseTexture(RHIDevice& device);

        std::string                     m_shader_path;
        bool                            m_enabled;
        float                           m_radius;
        int                             m_num_directions;
        int                             m_num_steps;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHITexture>     m_noise_texture;
        SceneColorSource*               m_scene_color {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
    };

} // namespace RealmEngine
