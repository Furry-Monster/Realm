#include "module/render/passes/spot_shadow_pass.h"

#include <algorithm>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>

#include "module/render/light.h"
#include "module/render/render_camera.h"
#include "module/render/render_scene.h"
#include "module/render/rhi/rhi_device.h"
#include "module/render/rhi/rhi_framebuffer.h"
#include "module/render/rhi/rhi_shader.h"

namespace RealmEngine
{
    SpotShadowPass::~SpotShadowPass() noexcept = default;

    SpotShadowPass::SpotShadowPass(const std::string& shader_path, const int resolution) :
        RenderPass("spot_shadow"), m_shader_path(shader_path), m_resolution(resolution)
    {}

    void SpotShadowPass::init(RHIDevice& device)
    {
        // Reuse the basic shadow shader for spot lights (perspective projection)
        m_shader = device.createShader(m_shader_path + "/builtin/shadow.vert", m_shader_path + "/builtin/shadow.frag");

        for (size_t i = 0; i < MAX_SPOT_SHADOWS; ++i)
        {
            FramebufferDesc desc;
            desc.width                       = m_resolution;
            desc.height                      = m_resolution;
            desc.has_depth                   = true;
            desc.depth_attachment.format     = TextureFormat::Depth24;
            desc.depth_attachment.min_filter = TextureFilter::Nearest;
            desc.depth_attachment.mag_filter = TextureFilter::Nearest;
            desc.depth_attachment.wrap       = TextureWrap::ClampToBorder;

            m_framebuffers.push_back(device.createFramebuffer(desc));
        }
    }

    void SpotShadowPass::execute(const RenderContext& ctx)
    {
        m_active_shadows.clear();

        struct Candidate
        {
            int       index;
            float     distance;
            glm::vec3 pos;
            glm::vec3 dir;
            float     outer_cone;
            float     range;
        };
        std::vector<Candidate> candidates;

        const auto&     lights  = ctx.scene->getLights();
        const glm::vec3 cam_pos = ctx.camera->getPosition();
        for (size_t i = 0; i < lights.size(); ++i)
        {
            const auto& light = lights[i];
            if (light.type != LightType::Spot)
                continue;
            const float dist = glm::length(light.position - cam_pos);
            candidates.push_back(
                {static_cast<int>(i), dist, light.position, light.direction, light.outer_cone_angle, light.range});
        }

        std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
            return a.distance < b.distance;
        });

        const size_t shadow_count = std::min(candidates.size(), MAX_SPOT_SHADOWS);

        for (size_t s = 0; s < shadow_count; ++s)
        {
            auto& c = candidates[s];

            const float     fov  = glm::radians(c.outer_cone * 2.0f);
            constexpr float near = 0.1f;
            const float     far  = c.range;
            glm::mat4       proj = glm::perspective(fov, 1.0f, near, far);

            glm::vec3 dir = glm::normalize(c.dir);
            glm::vec3 up  = glm::vec3(0.0f, 1.0f, 0.0f);
            if (std::abs(glm::dot(dir, up)) > 0.9f)
                up = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::mat4 view = glm::lookAt(c.pos, c.pos + dir, up);

            SpotShadowData sd;
            sd.light_index     = c.index;
            sd.light_view_proj = proj * view;
            sd.range           = c.range;
            m_active_shadows.push_back(sd);

            auto* fb = m_framebuffers[s].get();
            fb->bind();
            ctx.device->setViewport(0, 0, m_resolution, m_resolution);
            ctx.device->clear(ClearFlags::Depth);
            ctx.device->setCullFace(CullFace::Front);

            m_shader->use();
            m_shader->setMat4("lightSpaceMatrix", sd.light_view_proj);

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

        ctx.device->setCullFace(CullFace::Back);
        ctx.device->bindDefaultFramebuffer();
    }

    RHIFramebuffer* SpotShadowPass::getFramebuffer(const int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_framebuffers.size()))
            return m_framebuffers[static_cast<size_t>(index)].get();
        return nullptr;
    }

    void SpotShadowPass::dispose()
    {
        m_framebuffers.clear();
        m_shader.reset();
    }

} // namespace RealmEngine
