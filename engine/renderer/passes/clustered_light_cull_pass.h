#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIBuffer;

    class ClusteredLightCullPass final : public RenderPass
    {
    public:
        static constexpr int CLUSTER_X      = 16;
        static constexpr int CLUSTER_Y      = 9;
        static constexpr int CLUSTER_Z      = 24;
        static constexpr int TOTAL_CLUSTERS = CLUSTER_X * CLUSTER_Y * CLUSTER_Z;

        static constexpr uint32_t SSBO_BINDING_LIGHTS       = 1;
        static constexpr uint32_t SSBO_BINDING_CLUSTER_AABB = 2;
        static constexpr uint32_t SSBO_BINDING_LIGHT_INDEX  = 3;
        static constexpr uint32_t SSBO_BINDING_LIGHT_GRID   = 4;

        static constexpr int AVG_LIGHTS_PER_CLUSTER = 100;

        explicit ClusteredLightCullPass(const std::string& shader_path);
        ~ClusteredLightCullPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void buildClusterAABBs(const RenderContext& ctx);

        RHIBuffer* getLightBuffer() const { return m_light_ssbo.get(); }
        RHIBuffer* getClusterAABBBuffer() const { return m_cluster_aabb_ssbo.get(); }
        RHIBuffer* getLightIndexBuffer() const { return m_light_index_ssbo.get(); }
        RHIBuffer* getLightGridBuffer() const { return m_light_grid_ssbo.get(); }

    private:
        std::string m_shader_path;

        std::unique_ptr<RHIShader> m_build_shader;
        std::unique_ptr<RHIShader> m_cull_shader;

        std::unique_ptr<RHIBuffer> m_light_ssbo;
        std::unique_ptr<RHIBuffer> m_cluster_aabb_ssbo;
        std::unique_ptr<RHIBuffer> m_light_index_ssbo;
        std::unique_ptr<RHIBuffer> m_light_grid_ssbo;

        bool m_aabbs_built {false};
    };

} // namespace RealmEngine
