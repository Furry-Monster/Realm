#include "renderer/passes/ssao_blur_pass.h"

#include "renderer/fullscreen_quad.h"
#include "renderer/material.h"
#include "renderer/passes/ssao_pass.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    SSAOBlurPass::~SSAOBlurPass() = default;

    SSAOBlurPass::SSAOBlurPass(const std::string& shader_path) : RenderPass("ssao_blur"), m_shader_path(shader_path) {}

    void SSAOBlurPass::init(RHIDevice& device)
    {
        m_shader =
            device.createShader(m_shader_path + "/builtin/ssao_blur.vert", m_shader_path + "/builtin/ssao_blur.frag");
    }

    void SSAOBlurPass::execute(const RenderContext& ctx)
    {
        if (!m_ssao_pass || !m_ssao_pass->isEnabled() || !m_quad || !m_framebuffer)
            return;

        RHITexture* ssao_tex  = m_ssao_pass->getResultTexture();
        auto*       geo_fb    = m_scene_color ? m_scene_color->getFramebuffer() : nullptr;
        RHITexture* depth_tex = geo_fb ? geo_fb->getDepthAttachment() : nullptr;
        if (!ssao_tex || !depth_tex)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setDepthTest(false);
        ctx.device->clear(ClearFlags::Color);

        m_shader->use();

        ctx.device->bindTexture(TEXTURE_UNIT_SSAO_RESULT, *ssao_tex);
        m_shader->setInt("ssaoTexture", TEXTURE_UNIT_SSAO_RESULT);
        ctx.device->bindTexture(TEXTURE_UNIT_SSAO_DEPTH, *depth_tex);
        m_shader->setInt("depthTexture", TEXTURE_UNIT_SSAO_DEPTH);
        m_shader->setVec2(
            "texelSize",
            glm::vec2(1.0f / static_cast<float>(ctx.viewport_width), 1.0f / static_cast<float>(ctx.viewport_height)));

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void SSAOBlurPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

    RHITexture* SSAOBlurPass::getResultTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
