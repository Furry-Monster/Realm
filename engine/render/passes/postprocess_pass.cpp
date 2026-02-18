#include "render/passes/postprocess_pass.h"

#include "render/fullscreen_quad.h"
#include "render/passes/bloom_pass.h"
#include "render/passes/gtao_blur_pass.h"
#include "render/rhi/rhi_device.h"
#include "render/rhi/rhi_framebuffer.h"
#include "render/rhi/rhi_shader.h"
#include "render/rhi/rhi_texture.h"
#include "render/scene_color_source.h"

namespace RealmEngine
{
    PostProcessPass::~PostProcessPass() noexcept = default;

    PostProcessPass::PostProcessPass(const std::string& shader_path,
                                     const bool         tonemapping,
                                     const float        gamma,
                                     const bool         ao_enabled,
                                     const float        ao_power,
                                     const float        ao_intensity) :
        RenderPass("postprocess"), m_shader_path(shader_path), m_tonemapping(tonemapping), m_gamma(gamma),
        m_ao_enabled(ao_enabled), m_ao_power(ao_power), m_ao_intensity(ao_intensity)
    {}

    void PostProcessPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/post.vert", m_shader_path + "/builtin/post.frag");
    }

    void PostProcessPass::execute(const RenderContext& ctx)
    {
        if (!m_scene_color || !m_quad)
            return;

        auto* geo_fb = m_scene_color->getFramebuffer();
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

        bool        bloom_on        = m_bloom_pass && m_bloom_pass->isEnabled();
        const float bloom_intensity = bloom_on ? m_bloom_pass->getIntensity() : 0.0f;

        m_shader->setBool("tonemappingEnabled", m_tonemapping);
        m_shader->setFloat("gammaCorrectionFactor", m_gamma);

        auto* depth_tex = geo_fb->getDepthAttachment();

        RHITexture* ao_tex =
            (m_gtao_blur_pass && m_gtao_blur_pass->getResultTexture()) ? m_gtao_blur_pass->getResultTexture() : nullptr;
        const bool ao_on        = m_ao_enabled && ao_tex && depth_tex;
        int        display_mode = static_cast<int>(ctx.display_mode);
        if (display_mode == 7 && !ao_on)
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
        m_shader->setInt("bloomMaxMip", m_bloom_pass ? m_bloom_pass->getMaxMipLevel() : 0);

        m_shader->setBool("aoEnabled", ao_on);
        m_shader->setFloat("aoPower", m_ao_power);
        m_shader->setFloat("aoIntensity", m_ao_intensity);
        if (ao_on)
        {
            ctx.device->bindTexture(2, *ao_tex);
            m_shader->setInt("aoTexture", 2);
        }
        if (depth_tex && (ao_on || display_mode == 8))
        {
            ctx.device->bindTexture(3, *depth_tex);
            m_shader->setInt("depthTexture", 3);
        }

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void PostProcessPass::dispose() { m_shader.reset(); }

} // namespace RealmEngine
