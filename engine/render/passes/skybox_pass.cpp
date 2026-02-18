#include "render/passes/skybox_pass.h"

#include "render/render_camera.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_framebuffer.h"
#include "render/rhi/rhi_shader.h"
#include "render/scene_color_source.h"
#include "render/skybox.h"

namespace RealmEngine
{
    SkyboxPass::~SkyboxPass() = default;

    SkyboxPass::SkyboxPass(const std::string& shader_path) : RenderPass("skybox"), m_shader_path(shader_path) {}

    void SkyboxPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/skybox.vert", m_shader_path + "/builtin/skybox.frag");
    }

    void SkyboxPass::execute(const RenderContext& ctx)
    {
        if (!m_skybox)
            return;

        if (m_scene_color && m_scene_color->getFramebuffer())
            m_scene_color->getFramebuffer()->bind();

        ctx.device->setDepthFunc(DepthFunc::LessEqual);
        ctx.device->setDepthWrite(false);

        m_shader->use();
        constexpr glm::mat4 skybox_model = glm::mat4(1.0f);
        const glm::mat4     skybox_view  = glm::mat4(glm::mat3(ctx.camera->getViewMatrix()));
        const glm::mat4     skybox_proj  = ctx.camera->getProjMatrix();

        m_shader->setMVP(skybox_model, skybox_view, skybox_proj);
        m_shader->setInt("skybox", 0);

        m_skybox->draw(*ctx.device);

        ctx.device->setDepthWrite(true);
        ctx.device->setDepthFunc(DepthFunc::Less);
    }

    void SkyboxPass::dispose() { m_shader.reset(); }

} // namespace RealmEngine
