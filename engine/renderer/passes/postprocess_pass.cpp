#include "renderer/passes/postprocess_pass.h"

#include "renderer/fullscreen_quad.h"
#include "renderer/passes/bloom_pass.h"
#include "renderer/passes/geometry_pass.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    PostProcessPass::~PostProcessPass() = default;

    PostProcessPass::PostProcessPass(const std::string& shader_path, bool tonemapping, float gamma) :
        RenderPass("postprocess"), m_shader_path(shader_path), m_tonemapping(tonemapping), m_gamma(gamma)
    {}

    void PostProcessPass::initialize(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/post.vert", m_shader_path + "/post.frag");
    }

    void PostProcessPass::execute(const RenderContext& ctx)
    {
        if (!m_geometry_pass || !m_quad)
            return;

        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->bindDefaultFramebuffer();
        ctx.device->clear(ClearFlags::Color | ClearFlags::Depth);

        m_shader->use();

        bool  bloom_on        = m_bloom_pass && m_bloom_pass->isEnabled();
        float bloom_intensity = bloom_on ? m_bloom_pass->getIntensity() : 0.0f;

        m_shader->setBool("bloomEnabled", bloom_on);
        m_shader->setFloat("bloomIntensity", bloom_intensity);
        m_shader->setBool("tonemappingEnabled", m_tonemapping);
        m_shader->setFloat("gammaCorrectionFactor", m_gamma);

        // Main color texture
        auto* color_tex = m_geometry_pass->getFramebuffer()->getColorAttachment(0);
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
        }

        m_quad->draw();
    }

    void PostProcessPass::dispose() { m_shader.reset(); }

} // namespace RealmEngine
