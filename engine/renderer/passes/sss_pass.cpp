#include "renderer/passes/sss_pass.h"

#include "renderer/fullscreen_quad.h"
#include "renderer/render_camera.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace RealmEngine
{
    static constexpr int TEXTURE_UNIT_SSS_COLOR = 20;
    static constexpr int TEXTURE_UNIT_SSS_DEPTH = 21;

    SSSPass::~SSSPass() = default;

    SSSPass::SSSPass(const std::string& shader_path, bool enabled, float radius, int samples) :
        RenderPass("sss"), m_shader_path(shader_path), m_enabled(enabled), m_radius(std::max(radius, 0.01f)),
        m_samples(std::clamp(samples, 1, 9))
    {}

    void SSSPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/sss.vert", m_shader_path + "/sss.frag");
    }

    void SSSPass::execute(const RenderContext& ctx)
    {
        if (!m_enabled || !m_scene_color || !m_quad || !m_framebuffers[0] || !m_framebuffers[1])
            return;

        auto* geo_fb = m_scene_color->getFramebuffer();
        if (!geo_fb)
            return;

        RHITexture* color_tex = geo_fb->getColorAttachment(0);
        RHITexture* depth_tex = geo_fb->getDepthAttachment();
        if (!color_tex || !depth_tex)
            return;

        ctx.device->setDepthTest(false);
        m_shader->use();

        glm::mat4 proj     = ctx.camera->getProjMatrix();
        glm::mat4 inv_proj = glm::inverse(proj);
        m_shader->setMat4("projection", proj);
        m_shader->setMat4("invProjection", inv_proj);
        m_shader->setFloat("radius", m_radius);
        m_shader->setInt("samples", m_samples);

        ctx.device->bindTexture(TEXTURE_UNIT_SSS_COLOR, *color_tex);
        m_shader->setInt("colorTexture", TEXTURE_UNIT_SSS_COLOR);
        ctx.device->bindTexture(TEXTURE_UNIT_SSS_DEPTH, *depth_tex);
        m_shader->setInt("depthTexture", TEXTURE_UNIT_SSS_DEPTH);

        int w = m_framebuffers[0]->getWidth();
        int h = m_framebuffers[0]->getHeight();

        m_framebuffers[0]->bind();
        ctx.device->setViewport(0, 0, w, h);
        m_shader->setVec2("blurDirection", glm::vec2(1.0f, 0.0f));
        m_quad->draw();

        m_framebuffers[1]->bind();
        ctx.device->bindTexture(TEXTURE_UNIT_SSS_COLOR, *m_framebuffers[0]->getColorAttachment(0));
        m_shader->setVec2("blurDirection", glm::vec2(0.0f, 1.0f));
        m_quad->draw();

        ctx.device->blitFramebuffer(m_framebuffers[1].get(), geo_fb, 0, 0, w, h, 0, 0, w, h);

        m_result_idx = 1;
        ctx.device->setDepthTest(true);
    }

    void SSSPass::dispose()
    {
        m_shader.reset();
        m_framebuffers[0].reset();
        m_framebuffers[1].reset();
    }

    void SSSPass::setFramebuffers(std::unique_ptr<RHIFramebuffer> fb0, std::unique_ptr<RHIFramebuffer> fb1)
    {
        m_framebuffers[0] = std::move(fb0);
        m_framebuffers[1] = std::move(fb1);
    }

    RHITexture* SSSPass::getResultTexture() const
    {
        return m_framebuffers[m_result_idx] ? m_framebuffers[m_result_idx]->getColorAttachment(0) : nullptr;
    }

    RHIFramebuffer* SSSPass::getFramebuffer(uint32_t index) const { return m_framebuffers[index & 1].get(); }

} // namespace RealmEngine
