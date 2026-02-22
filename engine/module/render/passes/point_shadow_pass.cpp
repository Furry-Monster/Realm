#include "module/render/passes/point_shadow_pass.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "functional/render/light.h"
#include "functional/render/render_camera.h"
#include "module/render/render_scene.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_framebuffer.h"
#include "functional/render/rhi/rhi_shader.h"

namespace RealmEngine
{
    PointShadowPass::~PointShadowPass() noexcept = default;

    PointShadowPass::PointShadowPass(const std::string& shader_path, const int resolution) :
        RenderPass("point_shadow"), m_shader_path(shader_path), m_resolution(resolution)
    {}

    void PointShadowPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/shadow_point.vert",
                                       m_shader_path + "/builtin/shadow_point.geom",
                                       m_shader_path + "/builtin/shadow_point.frag");

        for (size_t i = 0; i < MAX_POINT_SHADOWS; ++i)
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
            color.format           = TextureFormat::R32F;
            color.min_filter       = TextureFilter::Nearest;
            color.mag_filter       = TextureFilter::Nearest;
            color.wrap             = TextureWrap::ClampToEdge;
            color.is_cubemap       = true;
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

        const auto&     lights  = ctx.scene->getLights();
        const glm::vec3 cam_pos = ctx.camera->getPosition();
        for (size_t i = 0; i < lights.size(); ++i)
        {
            const auto& light = lights[i];
            if (light.type != LightType::Point)
                continue;
            const float dist = glm::length(light.position - cam_pos);
            candidates.push_back({static_cast<int>(i), dist, light.position, light.range});
        }

        std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
            return a.distance < b.distance;
        });

        const size_t shadow_count = std::min(candidates.size(), MAX_POINT_SHADOWS);

        for (size_t s = 0; s < shadow_count; ++s)
        {
            const auto& c = candidates[s];

            PointShadowData sd;
            sd.light_index = c.index;
            sd.position    = c.pos;
            sd.range       = c.range;
            m_active_shadows.push_back(sd);

            constexpr float near = 0.1f;
            const float     far  = c.range;
            glm::mat4       proj = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
            glm::vec3       pos  = c.pos;

            // 6 face view matrices
            const glm::mat4 shadow_views[6] = {proj * glm::lookAt(pos, pos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
                                               proj * glm::lookAt(pos, pos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
                                               proj * glm::lookAt(pos, pos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
                                               proj * glm::lookAt(pos, pos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
                                               proj * glm::lookAt(pos, pos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
                                               proj * glm::lookAt(pos, pos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))};

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

                const auto& objects  = ctx.scene->getRenderObjects();
                const auto& matrices = ctx.scene->getRenderModelMatrices();
                for (size_t i = 0; i < objects.size(); ++i)
                {
                    auto&     ro    = *objects[i];
                    glm::mat4 model = (i < matrices.size()) ? matrices[i] : glm::mat4(1.0f);
                    m_shader->setMat4("model", model);
                    ro.drawShadow(*m_shader);
                }
            }
        }

        ctx.device->setCullFace(CullFace::Back);
        ctx.device->bindDefaultFramebuffer();
    }

    RHIFramebuffer* PointShadowPass::getFramebuffer(const int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_framebuffers.size()))
            return m_framebuffers[static_cast<size_t>(index)].get();
        return nullptr;
    }

    void PointShadowPass::dispose()
    {
        m_framebuffers.clear();
        m_shader.reset();
    }

} // namespace RealmEngine
