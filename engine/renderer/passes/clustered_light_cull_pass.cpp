#include "renderer/passes/clustered_light_cull_pass.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include <glm/glm.hpp>

#include "renderer/light.h"
#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "rhi/rhi_buffer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    ClusteredLightCullPass::ClusteredLightCullPass(const std::string& shader_path) :
        RenderPass("clustered_light_cull"), m_shader_path(shader_path)
    {}

    ClusteredLightCullPass::~ClusteredLightCullPass() = default;

    void ClusteredLightCullPass::init(RHIDevice& device)
    {
        m_build_shader = device.createComputeShader(m_shader_path + "/builtin/cluster_build.comp");
        m_cull_shader  = device.createComputeShader(m_shader_path + "/builtin/cluster_cull.comp");

        // binding 1: light data (count + MAX_LIGHTS * LightData)
        m_light_ssbo = device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);

        // binding 2: cluster AABBs (min + max per cluster, 2 * vec4)
        constexpr size_t aabb_size = static_cast<size_t>(TOTAL_CLUSTERS) * 2 * sizeof(glm::vec4);
        m_cluster_aabb_ssbo = device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, aabb_size);

        // binding 3: global light index list
        constexpr size_t index_list_size =
            static_cast<size_t>(TOTAL_CLUSTERS) * AVG_LIGHTS_PER_CLUSTER * sizeof(uint32_t);
        // +16 bytes at the front for the atomic counter
        m_light_index_ssbo = device.createBuffer(
            BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, sizeof(uint32_t) + index_list_size);

        // binding 4: per-cluster grid (offset, count) packed as uvec2
        constexpr size_t grid_size = static_cast<size_t>(TOTAL_CLUSTERS) * sizeof(glm::uvec2);
        m_light_grid_ssbo = device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, grid_size);
    }

    void ClusteredLightCullPass::buildClusterAABBs(const RenderContext& ctx)
    {
        m_build_shader->use();

        const glm::mat4 inv_proj = glm::inverse(ctx.camera->getProjMatrix());
        m_build_shader->setMat4("invProjection", inv_proj);
        m_build_shader->setFloat("nearPlane", ctx.camera->getNearPlane());
        m_build_shader->setFloat("farPlane", ctx.camera->getFarPlane());

        m_cluster_aabb_ssbo->bindBase(SSBO_BINDING_CLUSTER_AABB);

        ctx.device->dispatchCompute(
            static_cast<uint32_t>(CLUSTER_X), static_cast<uint32_t>(CLUSTER_Y), static_cast<uint32_t>(CLUSTER_Z));
        ctx.device->memoryBarrier(BarrierFlags::ShaderStorage);

        m_aabbs_built = true;
    }

    void ClusteredLightCullPass::execute(const RenderContext& ctx)
    {
        if (!m_aabbs_built)
            buildClusterAABBs(ctx);

        // Upload light data into SSBO
        {
            const size_t count   = std::min(ctx.scene->getLights().size(), MAX_LIGHTS);
            const int    count_i = static_cast<int>(count);

            std::vector<LightData> data(count);
            for (size_t i = 0; i < count; ++i)
            {
                auto& l             = ctx.scene->getLights()[i];
                data[i].position    = glm::vec4(l.position, static_cast<float>(static_cast<int>(l.type)));
                data[i].direction   = glm::vec4(l.direction, l.intensity);
                data[i].color       = glm::vec4(l.color, l.constant);
                data[i].attenuation = glm::vec4(l.linear, l.quadratic, l.range, l.inner_cone_angle);
                data[i].spot_area   = glm::vec4(l.outer_cone_angle, l.width, l.height, 0.0f);
            }

            m_light_ssbo->setSubData(&count_i, 0, sizeof(int));
            if (!data.empty())
                m_light_ssbo->setSubData(data.data(), 16, count * sizeof(LightData));
        }

        // Reset atomic counter in LightIndexBuffer to 0
        {
            constexpr uint32_t zero = 0;
            m_light_index_ssbo->setSubData(&zero, 0, sizeof(uint32_t));
        }

        // Clear grid to zero
        {
            const std::vector<glm::uvec2> empty_grid(TOTAL_CLUSTERS, glm::uvec2(0));
            m_light_grid_ssbo->setSubData(empty_grid.data(), 0, TOTAL_CLUSTERS * sizeof(glm::uvec2));
        }

        // Bind all SSBOs
        m_light_ssbo->bindBase(SSBO_BINDING_LIGHTS);
        m_cluster_aabb_ssbo->bindBase(SSBO_BINDING_CLUSTER_AABB);
        m_light_index_ssbo->bindBase(SSBO_BINDING_LIGHT_INDEX);
        m_light_grid_ssbo->bindBase(SSBO_BINDING_LIGHT_GRID);

        // Dispatch culling: one workgroup per Z-slice, each workgroup covers X*Y clusters
        m_cull_shader->use();

        const glm::mat4 view = ctx.camera->getViewMatrix();
        m_cull_shader->setMat4("viewMatrix", view);

        ctx.device->dispatchCompute(1, 1, static_cast<uint32_t>(CLUSTER_Z));
        ctx.device->memoryBarrier(BarrierFlags::ShaderStorage);
    }

    void ClusteredLightCullPass::dispose()
    {
        m_light_ssbo.reset();
        m_cluster_aabb_ssbo.reset();
        m_light_index_ssbo.reset();
        m_light_grid_ssbo.reset();
        m_build_shader.reset();
        m_cull_shader.reset();
        m_aabbs_built = false;
    }

} // namespace RealmEngine
