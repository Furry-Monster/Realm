#include "renderer/passes/point_shadow_pass.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/light.h"
#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    PointShadowPass::~PointShadowPass() = default;

    PointShadowPass::PointShadowPass(const std::string& shader_path, int resolution) :
        RenderPass("point_shadow"), m_shader_path(shader_path), m_resolution(resolution)
    {}

    void PointShadowPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/shadow_point.vert",
                                       m_shader_path + "/builtin/shadow_point.geom",
                                       m_shader_path + "/builtin/shadow_point.frag");

        for (int i = 0; i < MAX_POINT_SHADOWS; ++i)
        {
            FramebufferDesc desc;
            desc.width  = m_resolution;
            desc.height = m_resolution;

            FramebufferAttachment depth;
            depth.format     = TextureFormat::Depth24;
            depth.min_filter = TextureFilter::Nearest;
            depth.mag_filter = TextureFilter::Nearest;
            depth.wrap       = TextureWrap::ClampToEdge;
            depth.is_cubemap = true;

            desc.has_depth        = true;
            desc.depth_attachment = depth;

            // Need a color attachment as cubemap to satisfy the FBO completeness
            // Actually for depth cubemap, we use a special approach:
            // Use a cubemap color attachment and write linear depth in fragment shader
            FramebufferAttachment color;
            color.format     = TextureFormat::R32F;
            color.min_filter = TextureFilter::Nearest;
            color.mag_filter = TextureFilter::Nearest;
            color.wrap       = TextureWrap::ClampToEdge;
            color.is_cubemap = true;
            desc.color_attachments = {color};

            m_framebuffers.push_back(device.createFramebuffer(desc));
        }
    }

    void PointShadowPass::execute(const RenderContext& ctx)
    {
        m_active_shadows.clear();

        // Collect point lights that need shadows (nearest N)
        struct Candidate
        {
            int       index;
            float     distance;
            glm::vec3 pos;
            float     range;
        };
        std::vector<Candidate> candidates;

        glm::vec3 cam_pos = ctx.camera->getPosition();
        for (size_t i = 0; i < ctx.scene->m_lights.size(); ++i)
        {
            auto& light = ctx.scene->m_lights[i];
            if (light.type != LightType::Point)
                continue;
            float dist = glm::length(light.position - cam_pos);
            candidates.push_back({static_cast<int>(i), dist, light.position, light.range});
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) { return a.distance < b.distance; });

        int shadow_count = std::min(static_cast<int>(candidates.size()), MAX_POINT_SHADOWS);

        for (int s = 0; s < shadow_count; ++s)
        {
            auto& c = candidates[s];

            PointShadowData sd;
            sd.light_index = c.index;
            sd.position    = c.pos;
            sd.range       = c.range;
            m_active_shadows.push_back(sd);

            float     near    = 0.1f;
            float     far     = c.range;
            glm::mat4 proj    = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
            glm::vec3 pos     = c.pos;

            // 6 face view matrices
            glm::mat4 shadow_views[6] = {
                proj * glm::lookAt(pos, pos + glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)),
                proj * glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)),
                proj * glm::lookAt(pos, pos + glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)),
                proj * glm::lookAt(pos, pos + glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)),
                proj * glm::lookAt(pos, pos + glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)),
                proj * glm::lookAt(pos, pos + glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0))
            };

            auto* fb = m_framebuffers[s].get();

            m_shader->use();
            for (int face = 0; face < 6; ++face)
                m_shader->setMat4("shadowMatrices[" + std::to_string(face) + "]", shadow_views[face]);
            m_shader->setVec3("lightPos", pos);
            m_shader->setFloat("farPlane", far);

            for (int face = 0; face < 6; ++face)
            {
                fb->setCubeFace(face);
                fb->bind();
                ctx.device->setViewport(0, 0, m_resolution, m_resolution);
                ctx.device->clear(ClearFlags::Color | ClearFlags::Depth);
                ctx.device->setCullFace(CullFace::Front);

                for (size_t i = 0; i < ctx.scene->m_render_objects.size(); ++i)
                {
                    auto&     ro    = ctx.scene->m_render_objects[i];
                    glm::mat4 model = (i < ctx.scene->m_render_model_matrices.size())
                                          ? ctx.scene->m_render_model_matrices[i]
                                          : glm::mat4(1.0f);
                    m_shader->setMat4("model", model);
                    ro->drawShadow(*m_shader);
                }
            }
        }

        ctx.device->setCullFace(CullFace::Back);
        ctx.device->bindDefaultFramebuffer();
    }

    RHIFramebuffer* PointShadowPass::getFramebuffer(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_framebuffers.size()))
            return m_framebuffers[index].get();
        return nullptr;
    }

    void PointShadowPass::dispose()
    {
        m_framebuffers.clear();
        m_shader.reset();
    }

} // namespace RealmEngine
