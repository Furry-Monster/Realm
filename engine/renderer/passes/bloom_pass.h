#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class FullscreenQuad;
    class GeometryPass;

    class BloomPass final : public RenderPass
    {
    public:
        BloomPass(const std::string& shader_path,
                  bool               enabled,
                  float              intensity,
                  int                iterations,
                  BloomDirection     direction);
        ~BloomPass() override;

        void initialize(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setGeometryPass(GeometryPass* gp) { m_geometry_pass = gp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffers(std::unique_ptr<RHIFramebuffer> fb0, std::unique_ptr<RHIFramebuffer> fb1);

        RHITexture* getResultTexture() const;
        bool        isEnabled() const { return m_enabled; }
        float       getIntensity() const { return m_intensity; }

    private:
        std::string    m_shader_path;
        bool           m_enabled;
        float          m_intensity;
        int            m_iterations;
        BloomDirection m_direction;

        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffers[2];
        GeometryPass*                   m_geometry_pass {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
        uint32_t                        m_result_idx {0};
    };

} // namespace RealmEngine
