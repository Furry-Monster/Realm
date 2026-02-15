#include "renderer/passes/postprocess_pass.h"

#include "renderer/fullscreen_quad.h"
#include "renderer/passes/bloom_pass.h"
#include "renderer/passes/geometry_pass.h"
#include "renderer/passes/ssao_blur_pass.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    PostProcessPass::~PostProcessPass() = default;

    PostProcessPass::PostProcessPass(const std::string& shader_path,
                                     bool               tonemapping,
                                     float              gamma,
                                     bool               ssao_enabled,
                                     float              ssao_power) :
        RenderPass("postprocess"), m_shader_path(shader_path), m_tonemapping(tonemapping), m_gamma(gamma),
        m_ssao_enabled(ssao_enabled), m_ssao_power(ssao_power)
    {}

    void PostProcessPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/post.vert", m_shader_path + "/post.frag");
    }

    void PostProcessPass::execute(const RenderContext& ctx)
    {
        if (!m_geometry_pass || !m_quad)
            return;

        auto* geo_fb = m_geometry_pass->getFramebuffer();
        if (!geo_fb)
            return;

        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        if (ctx.viewport_framebuffer)
            ctx.viewport_framebuffer->bind();
        else
            ctx.device->bindDefaultFramebuffer();
        ctx.device->setDepthTest(false);
        ctx.device->clear(ClearFlags::Color | ClearFlags::Depth);

        m_shader->use();

        bool  bloom_on        = m_bloom_pass && m_bloom_pass->isEnabled();
        float bloom_intensity = bloom_on ? m_bloom_pass->getIntensity() : 0.0f;

        m_shader->setBool("tonemappingEnabled", m_tonemapping);
        m_shader->setFloat("gammaCorrectionFactor", m_gamma);

        auto* depth_tex = geo_fb->getDepthAttachment();
        bool  ssao_on   = m_ssao_enabled && m_ssao_blur_pass && m_ssao_blur_pass->getResultTexture() && depth_tex;
        int   display_mode = static_cast<int>(ctx.display_mode);
        if (display_mode == 7 && !ssao_on)
            display_mode = 0;
        m_shader->setInt("displayMode", display_mode);

        // Main color texture
        auto* color_tex = geo_fb->getColorAttachment(0);
        if (color_tex)
        {
            ctx.device->bindTexture(0, *color_tex);
            m_shader->setInt("colorTexture", 0);
        }

        // Bloom texture
        if (bloom_on)
        {
            auto* bloom_tex = m_bloom_pass->getResultTexture();
            if (bloom_tex)
            {
                ctx.device->bindTexture(1, *bloom_tex);
                m_shader->setInt("bloomTexture", 1);
            }
            else
            {
                bloom_on = false; // no valid bloom result, disable in shader
            }
        }

        m_shader->setBool("bloomEnabled", bloom_on);
        m_shader->setFloat("bloomIntensity", bloom_intensity);

        m_shader->setBool("ssaoEnabled", ssao_on);
        m_shader->setFloat("ssaoPower", m_ssao_power);
        if (ssao_on)
        {
            ctx.device->bindTexture(2, *m_ssao_blur_pass->getResultTexture());
            m_shader->setInt("ssaoTexture", 2);
        }
        if (depth_tex && (ssao_on || display_mode == 8))
        {
            ctx.device->bindTexture(3, *depth_tex);
            m_shader->setInt("depthTexture", 3);
        }

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void PostProcessPass::dispose() { m_shader.reset(); }

} // namespace RealmEngine
