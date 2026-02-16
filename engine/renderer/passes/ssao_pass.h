#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class SceneColorSource;
    class FullscreenQuad;

    class SSAOPass final : public RenderPass
    {
    public:
        SSAOPass(const std::string& shader_path,
                 bool               enabled,
                 float              radius,
                 float              bias,
                 int                kernel_size,
                 int                noise_size);
        ~SSAOPass() override;

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
        void generateKernel();
        void generateNoiseTexture(RHIDevice& device);

        std::string                     m_shader_path;
        bool                            m_enabled;
        float                           m_radius;
        float                           m_bias;
        int                             m_kernel_size;
        int                             m_noise_size;
        std::vector<glm::vec3>          m_kernel;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        std::unique_ptr<RHITexture>     m_noise_texture;
        SceneColorSource*               m_scene_color {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
    };

} // namespace RealmEngine
