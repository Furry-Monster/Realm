#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "functional/render/render_pass.h"
#include "functional/render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;

    struct SpotShadowData
    {
        int       light_index {-1};
        glm::mat4 light_view_proj {1.0f};
        float     range {25.0f};
    };

    class SpotShadowPass final : public RenderPass
    {
    public:
        static constexpr size_t MAX_SPOT_SHADOWS = 4;

        explicit SpotShadowPass(const std::string& shader_path, int resolution = 1024);
        ~SpotShadowPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        int getActiveShadowCount() const { return static_cast<int>(m_active_shadows.size()); }
        const std::vector<SpotShadowData>& getActiveShadows() const { return m_active_shadows; }
        RHIFramebuffer*                    getFramebuffer(int index) const;

    private:
        std::string                                  m_shader_path;
        int                                          m_resolution;
        std::unique_ptr<RHIShader>                   m_shader;
        std::vector<std::unique_ptr<RHIFramebuffer>> m_framebuffers;
        std::vector<SpotShadowData>                  m_active_shadows;
    };

} // namespace RealmEngine
