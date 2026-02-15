#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class GeometryPass;
    class FullscreenQuad;

    class SSSPass final : public RenderPass
    {
    public:
        SSSPass(const std::string& shader_path, bool enabled, float radius, int samples);
        ~SSSPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setGeometryPass(GeometryPass* gp) { m_geometry_pass = gp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffers(std::unique_ptr<RHIFramebuffer> fb0, std::unique_ptr<RHIFramebuffer> fb1);

        bool            isEnabled() const { return m_enabled; }
        RHITexture*     getResultTexture() const;
        RHIFramebuffer* getFramebuffer(uint32_t index) const;

    private:
        std::string                     m_shader_path;
        bool                            m_enabled;
        float                           m_radius;
        int                             m_samples;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffers[2];
        uint32_t                        m_result_idx {0};
        GeometryPass*                   m_geometry_pass {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
    };

} // namespace RealmEngine
