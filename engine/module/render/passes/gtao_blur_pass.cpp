#include "module/render/passes/gtao_blur_pass.h"

#include "functional/render/fullscreen_quad.h"
#include "functional/render/material.h"
#include "module/render/passes/gtao_pass.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_framebuffer.h"
#include "functional/render/rhi/rhi_shader.h"
#include "functional/render/rhi/rhi_texture.h"
#include "functional/render/scene_color_source.h"

namespace RealmEngine
{
    GTAOBlurPass::~GTAOBlurPass() noexcept = default;

    GTAOBlurPass::GTAOBlurPass(const std::string& shader_path) : RenderPass("gtao_blur"), m_shader_path(shader_path) {}

    void GTAOBlurPass::init(RHIDevice& device)
    {
        m_shader =
            device.createShader(m_shader_path + "/builtin/gtao_blur.vert", m_shader_path + "/builtin/gtao_blur.frag");
    }

    void GTAOBlurPass::execute(const RenderContext& ctx)
    {
        if (!m_gtao_pass || !m_gtao_pass->isEnabled() || !m_quad || !m_framebuffer)
            return;

        RHITexture* gtao_tex  = m_gtao_pass->getResultTexture();
        auto*       geo_fb    = m_scene_color ? m_scene_color->getFramebuffer() : nullptr;
        RHITexture* depth_tex = geo_fb ? geo_fb->getDepthAttachment() : nullptr;
        if (!gtao_tex || !depth_tex)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setDepthTest(false);
        ctx.device->clear(ClearFlags::Color);

        m_shader->use();

        ctx.device->bindTexture(TEXTURE_UNIT_GTAO_RESULT, *gtao_tex);
        m_shader->setInt("gtaoTexture", TEXTURE_UNIT_GTAO_RESULT);
        ctx.device->bindTexture(TEXTURE_UNIT_GTAO_DEPTH, *depth_tex);
        m_shader->setInt("depthTexture", TEXTURE_UNIT_GTAO_DEPTH);
        m_shader->setVec2(
            "texelSize",
            glm::vec2(1.0f / static_cast<float>(ctx.viewport_width), 1.0f / static_cast<float>(ctx.viewport_height)));

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void GTAOBlurPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

    RHITexture* GTAOBlurPass::getResultTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
