#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class SSAOPass;
    class GeometryPass;
    class FullscreenQuad;

    class SSAOBlurPass final : public RenderPass
    {
    public:
        explicit SSAOBlurPass(const std::string& shader_path);
        ~SSAOBlurPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSSAOPass(SSAOPass* pass) { m_ssao_pass = pass; }
        void setGeometryPass(GeometryPass* gp) { m_geometry_pass = gp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffer(std::unique_ptr<RHIFramebuffer> fb) { m_framebuffer = std::move(fb); }

        RHITexture* getResultTexture() const;

    private:
        std::string                     m_shader_path;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        SSAOPass*                       m_ssao_pass {nullptr};
        GeometryPass*                   m_geometry_pass {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
    };

} // namespace RealmEngine
