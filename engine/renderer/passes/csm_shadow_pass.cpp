#include "renderer/passes/csm_shadow_pass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <cmath>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <optional>

#include "renderer/light.h"
#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    CSMShadowPass::~CSMShadowPass() = default;

    CSMShadowPass::CSMShadowPass(const std::string& shader_path, int resolution) :
        RenderPass("csm_shadow"), m_shader_path(shader_path), m_resolution(resolution)
    {}

    void CSMShadowPass::init(RHIDevice& device)
    {
        m_shader =
            device.createShader(m_shader_path + "/builtin/shadow.vert", m_shader_path + "/builtin/shadow.frag");

        FramebufferDesc desc;
        desc.width                       = m_resolution;
        desc.height                      = m_resolution;
        desc.has_depth                   = true;
        desc.depth_attachment.format     = TextureFormat::Depth24;
        desc.depth_attachment.min_filter = TextureFilter::Nearest;
        desc.depth_attachment.mag_filter = TextureFilter::Nearest;
        desc.depth_attachment.wrap       = TextureWrap::ClampToBorder;
        desc.depth_attachment.is_array   = true;
        desc.depth_attachment.layers     = CASCADE_COUNT;

        m_framebuffer = device.createFramebuffer(desc);
    }

    void CSMShadowPass::computeCascadeSplits(float near_plane, float far_plane)
    {
        float range = far_plane - near_plane;
        float ratio = far_plane / near_plane;

        m_split_distances[0] = near_plane;
        for (int i = 1; i <= CASCADE_COUNT; ++i)
        {
            float p    = static_cast<float>(i) / static_cast<float>(CASCADE_COUNT);
            float log  = near_plane * std::pow(ratio, p);
            float lin  = near_plane + range * p;
            float dist = m_split_lambda * log + (1.0f - m_split_lambda) * lin;
            m_split_distances[i] = dist;
        }
    }

    void CSMShadowPass::computeCascadeMatrix(int cascade_index, const glm::mat4& view, const glm::mat4& proj,
                                             const glm::vec3& light_dir)
    {
        float near_split = m_split_distances[cascade_index];
        float far_split  = m_split_distances[cascade_index + 1];

        // Build sub-frustum projection
        glm::mat4 inv_vp = glm::inverse(proj * view);

        // NDC corners of the sub-frustum
        // We need to remap near_split / far_split to NDC z
        // For perspective: z_ndc = (2*near*far)/(far-near)/z - (far+near)/(far-near)
        // Simpler: build a sub-projection matrix and invert
        // Actually, we extract frustum corners in world space directly.

        // Frustum corners in NDC
        static constexpr glm::vec3 ndc_corners[8] = {
            {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
            {-1, -1,  1}, {1, -1,  1}, {1,  1,  1}, {-1,  1,  1}
        };

        // Full frustum corners in world space
        glm::vec3 world_corners[8];
        for (int i = 0; i < 8; ++i)
        {
            glm::vec4 h = inv_vp * glm::vec4(ndc_corners[i], 1.0f);
            world_corners[i] = glm::vec3(h) / h.w;
        }

        // Interpolate near/far planes to get sub-frustum
        // Near corners: 0-3, Far corners: 4-7
        float near_plane = m_split_distances[0];
        float far_plane  = m_split_distances[CASCADE_COUNT];
        float near_ratio = (near_split - near_plane) / (far_plane - near_plane);
        float far_ratio  = (far_split - near_plane) / (far_plane - near_plane);

        glm::vec3 sub_corners[8];
        for (int i = 0; i < 4; ++i)
        {
            glm::vec3 ray = world_corners[i + 4] - world_corners[i];
            sub_corners[i]     = world_corners[i] + ray * near_ratio;
            sub_corners[i + 4] = world_corners[i] + ray * far_ratio;
        }

        // Compute center of sub-frustum
        glm::vec3 center(0.0f);
        for (int i = 0; i < 8; ++i)
            center += sub_corners[i];
        center /= 8.0f;

        // Light view matrix
        glm::vec3 light_up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(light_dir, light_up)) > 0.9f)
            light_up = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::mat4 light_view = glm::lookAt(center - light_dir, center, light_up);

        // Compute AABB of sub-frustum in light space
        float min_x = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();
        float min_z = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::lowest();

        for (int i = 0; i < 8; ++i)
        {
            glm::vec3 ls = glm::vec3(light_view * glm::vec4(sub_corners[i], 1.0f));
            min_x = std::min(min_x, ls.x);
            max_x = std::max(max_x, ls.x);
            min_y = std::min(min_y, ls.y);
            max_y = std::max(max_y, ls.y);
            min_z = std::min(min_z, ls.z);
            max_z = std::max(max_z, ls.z);
        }

        // Extend z range to avoid clipping shadow casters behind the frustum
        static constexpr float Z_MARGIN = 50.0f;
        min_z -= Z_MARGIN;

        glm::mat4 light_proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);

        // Texel snapping: round the origin to shadow map texel grid
        glm::mat4 shadow_matrix   = light_proj * light_view;
        glm::vec4 origin_ndc      = shadow_matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float     texel_size      = 2.0f * (max_x - min_x) / static_cast<float>(m_resolution);
        glm::vec4 origin_rounded  = origin_ndc;
        origin_rounded.x           = std::round(origin_ndc.x / texel_size) * texel_size;
        origin_rounded.y          = std::round(origin_ndc.y / texel_size) * texel_size;
        glm::vec4 offset          = origin_rounded - origin_ndc;
        light_proj[3][0] += offset.x;
        light_proj[3][1] += offset.y;

        m_cascades[cascade_index].light_view_proj = light_proj * light_view;
        m_cascades[cascade_index].split_depth     = far_split;
    }

    void CSMShadowPass::execute(const RenderContext& ctx)
    {
        auto directional_light = ctx.scene->findDirectionalLight();
        if (!directional_light.has_value())
        {
            m_shadow_enabled = false;
            return;
        }

        m_shadow_enabled = true;
        const Light& light = directional_light->get();
        glm::vec3 light_dir = glm::normalize(light.direction);

        computeCascadeSplits(ctx.camera->getNearPlane(), ctx.camera->getFarPlane());

        for (int c = 0; c < CASCADE_COUNT; ++c)
        {
            computeCascadeMatrix(c, ctx.camera->getViewMatrix(), ctx.camera->getProjMatrix(), light_dir);
        }

        m_framebuffer->bind();
        ctx.device->setCullFace(CullFace::Front);

        for (int c = 0; c < CASCADE_COUNT; ++c)
        {
            m_framebuffer->setLayer(c);
            ctx.device->setViewport(0, 0, m_resolution, m_resolution);
            ctx.device->clear(ClearFlags::Depth);

            m_shader->use();
            m_shader->setMat4("lightSpaceMatrix", m_cascades[c].light_view_proj);

            const auto& objects = ctx.scene->getRenderObjects();
            const auto& matrices = ctx.scene->getRenderModelMatrices();
            for (size_t i = 0; i < objects.size(); ++i)
            {
                auto&     ro    = *objects[i];
                glm::mat4 model = (i < matrices.size()) ? matrices[i]
                                      : glm::mat4(1.0f);
                m_shader->setMat4("model", model);
                ro.drawShadow(*m_shader);
            }
        }

        ctx.device->setCullFace(CullFace::Back);
        ctx.device->bindDefaultFramebuffer();
    }

    void CSMShadowPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

} // namespace RealmEngine
