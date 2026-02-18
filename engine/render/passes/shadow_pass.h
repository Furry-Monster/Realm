#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "render/render_pass.h"
#include "render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;

    class ShadowPass final : public RenderPass
    {
    public:
        explicit ShadowPass(const std::string& shader_path, int resolution = 2048);
        ~ShadowPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        bool             isShadowEnabled() const { return m_shadow_enabled; }
        const glm::mat4& getLightSpaceMatrix() const { return m_light_space_matrix; }
        RHIFramebuffer*  getFramebuffer() const { return m_framebuffer.get(); }

    private:
        std::string                     m_shader_path;
        int                             m_resolution;
        std::unique_ptr<RHIShader>      m_shader;
        std::unique_ptr<RHIFramebuffer> m_framebuffer;
        glm::mat4                       m_light_space_matrix {1.0f};
        bool                            m_shadow_enabled {false};
    };

} // namespace RealmEngine
