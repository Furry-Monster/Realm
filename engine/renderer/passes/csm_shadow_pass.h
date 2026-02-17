#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "renderer/render_pass.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;

    class CSMShadowPass final : public RenderPass
    {
    public:
        static constexpr int CASCADE_COUNT = 4;

        struct CascadeData
        {
            glm::mat4 light_view_proj {1.0f};
            float     split_depth {0.0f}; // far plane of this cascade in view space
        };

        explicit CSMShadowPass(const std::string& shader_path, int resolution_per_cascade = 2048);
        ~CSMShadowPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        bool                                          isShadowEnabled() const { return m_shadow_enabled; }
        const std::array<CascadeData, CASCADE_COUNT>& getCascades() const { return m_cascades; }
        RHIFramebuffer*                               getFramebuffer() const { return m_framebuffer.get(); }
        float                                         getLightSize() const { return m_light_size; }
        void                                          setLightSize(float size) { m_light_size = size; }

    private:
        void computeCascadeSplits(float near_plane, float far_plane);
        void computeCascadeMatrix(int              cascade_index,
                                  const glm::mat4& view,
                                  const glm::mat4& proj,
                                  const glm::vec3& light_dir);

        std::string m_shader_path;
        int         m_resolution;
        float       m_light_size {1.0f};
        // lambda for log-linear split: 0 = linear, 1 = logarithmic
        float                                  m_split_lambda {0.5f};
        std::unique_ptr<RHIShader>             m_shader;
        std::unique_ptr<RHIFramebuffer>        m_framebuffer;
        std::array<CascadeData, CASCADE_COUNT> m_cascades {};
        std::array<float, CASCADE_COUNT + 1>   m_split_distances {};
        bool                                   m_shadow_enabled {false};
    };

} // namespace RealmEngine
