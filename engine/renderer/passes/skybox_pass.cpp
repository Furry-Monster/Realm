#include "renderer/passes/skybox_pass.h"

#include "renderer/passes/geometry_pass.h"
#include "renderer/render_camera.h"
#include "renderer/skybox.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    SkyboxPass::~SkyboxPass() = default;

    SkyboxPass::SkyboxPass(const std::string& shader_path) : RenderPass("skybox"), m_shader_path(shader_path) {}

    void SkyboxPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/skybox.vert", m_shader_path + "/skybox.frag");
    }

    void SkyboxPass::execute(const RenderContext& ctx)
    {
        if (!m_skybox)
            return;

        // Explicitly bind geometry framebuffer (skybox renders into the same FBO)
        if (m_geometry_pass && m_geometry_pass->getFramebuffer())
            m_geometry_pass->getFramebuffer()->bind();

        ctx.device->setDepthFunc(DepthFunc::LessEqual);
        ctx.device->setDepthWrite(false);

        m_shader->use();
        glm::mat4 skybox_model = glm::mat4(1.0f);
        glm::mat4 skybox_view  = glm::mat4(glm::mat3(ctx.camera->getViewMatrix()));
        glm::mat4 skybox_proj  = ctx.camera->getProjMatrix();

        m_shader->setMVP(skybox_model, skybox_view, skybox_proj);
        m_shader->setInt("skybox", 0);

        m_skybox->draw(*ctx.device);

        ctx.device->setDepthWrite(true);
        ctx.device->setDepthFunc(DepthFunc::Less);
    }

    void SkyboxPass::dispose() { m_shader.reset(); }

} // namespace RealmEngine
