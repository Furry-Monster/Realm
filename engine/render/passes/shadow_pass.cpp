#include "render/passes/shadow_pass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "render/light.h"
#include "render/render_camera.h"
#include "render/render_scene.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_framebuffer.h"
#include "render/rhi/rhi_shader.h"

namespace RealmEngine
{
    ShadowPass::~ShadowPass() = default;

    ShadowPass::ShadowPass(const std::string& shader_path, const int resolution) :
        RenderPass("shadow"), m_shader_path(shader_path), m_resolution(resolution)
    {}

    void ShadowPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/shadow.vert", m_shader_path + "/builtin/shadow.frag");

        FramebufferDesc desc;
        desc.width                       = m_resolution;
        desc.height                      = m_resolution;
        desc.has_depth                   = true;
        desc.depth_attachment.format     = TextureFormat::Depth24;
        desc.depth_attachment.min_filter = TextureFilter::Nearest;
        desc.depth_attachment.mag_filter = TextureFilter::Nearest;
        desc.depth_attachment.wrap       = TextureWrap::ClampToBorder;
        // No color attachments (depth-only)

        m_framebuffer = device.createFramebuffer(desc);
    }

    void ShadowPass::execute(const RenderContext& ctx)
    {
        const auto directional_light = ctx.scene->findDirectionalLight();
        if (!directional_light.has_value())
        {
            m_shadow_enabled = false;
            return;
        }

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, m_resolution, m_resolution);
        ctx.device->clear(ClearFlags::Depth);
        ctx.device->setCullFace(CullFace::Front);

        m_shader->use();

        static constexpr float SHADOW_NEAR    = 0.1f;
        static constexpr float SHADOW_FAR     = 50.0f;
        static constexpr float SHADOW_FRUSTUM = 20.0f;

        const Light&    light = directional_light->get();
        const glm::mat4 light_proj =
            glm::ortho(-SHADOW_FRUSTUM, SHADOW_FRUSTUM, -SHADOW_FRUSTUM, SHADOW_FRUSTUM, SHADOW_NEAR, SHADOW_FAR);
        const glm::vec3 light_dir    = glm::normalize(light.direction);
        const glm::vec3 scene_center = ctx.camera->getPosition();
        glm::vec3       light_up     = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(light_dir, light_up)) > 0.9f)
            light_up = glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::mat4 light_view =
            glm::lookAt(scene_center - light_dir * SHADOW_FRUSTUM * 0.5f, scene_center, light_up);

        m_light_space_matrix = light_proj * light_view;
        m_shadow_enabled     = true;

        const auto& objects  = ctx.scene->getRenderObjects();
        const auto& matrices = ctx.scene->getRenderModelMatrices();
        for (size_t i = 0; i < objects.size(); ++i)
        {
            auto&     ro    = *objects[i];
            glm::mat4 model = (i < matrices.size()) ? matrices[i] : glm::mat4(1.0f);

            m_shader->setMat4("lightSpaceMatrix", m_light_space_matrix);
            m_shader->setMat4("model", model);
            ro.drawShadow(*m_shader);
        }

        ctx.device->setCullFace(CullFace::Back);
        ctx.device->bindDefaultFramebuffer();
    }

    void ShadowPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

} // namespace RealmEngine
