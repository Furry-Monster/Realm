#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "render/render_pass.h"
#include "render/rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;

    struct PointShadowData
    {
        int       light_index {-1};
        glm::vec3 position {0.0f};
        float     range {25.0f};
    };

    class PointShadowPass final : public RenderPass
    {
    public:
        static constexpr size_t MAX_POINT_SHADOWS = 4;

        explicit PointShadowPass(const std::string& shader_path, int resolution = 1024);
        ~PointShadowPass() noexcept override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        int getActiveShadowCount() const { return static_cast<int>(m_active_shadows.size()); }
        const std::vector<PointShadowData>& getActiveShadows() const { return m_active_shadows; }
        RHIFramebuffer*                     getFramebuffer(int index) const;

    private:
        std::string                                  m_shader_path;
        int                                          m_resolution;
        std::unique_ptr<RHIShader>                   m_shader;
        std::vector<std::unique_ptr<RHIFramebuffer>> m_framebuffers;
        std::vector<PointShadowData>                 m_active_shadows;
    };

} // namespace RealmEngine
