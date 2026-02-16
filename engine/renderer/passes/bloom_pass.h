#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    // Bloom blur direction -- renderer-layer concept, not part of RHI.
    enum class BloomDirection : uint8_t
    {
        BOTH       = 0,
        HORIZONTAL = 1,
        VERTICAL   = 2
    };
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class FullscreenQuad;
    class SceneColorSource;

    class BloomPass final : public RenderPass
    {
    public:
        BloomPass(const std::string& shader_path,
                  bool               enabled,
                  float              intensity,
                  float              brightness_cutoff,
                  int                iterations,
                  BloomDirection     direction);
        ~BloomPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }
        void setFramebuffers(std::unique_ptr<RHIFramebuffer> fb0, std::unique_ptr<RHIFramebuffer> fb1);

        RHITexture* getResultTexture() const;
        int         getMaxMipLevel() const;
        bool        isEnabled() const { return m_enabled; }
        float       getIntensity() const { return m_intensity; }

    private:
        std::string    m_shader_path;
        bool           m_enabled;
        float          m_intensity;
        float          m_brightness_cutoff;
        int            m_iterations;
        BloomDirection m_direction;

        std::unique_ptr<RHIShader>      m_extract_shader;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffers[2];
        SceneColorSource*               m_scene_color {nullptr};
        FullscreenQuad*                 m_quad {nullptr};
        uint32_t                        m_result_idx {0};
    };

} // namespace RealmEngine
