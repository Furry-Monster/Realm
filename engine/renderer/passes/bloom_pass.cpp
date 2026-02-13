#include "renderer/passes/bloom_pass.h"

#include "renderer/fullscreen_quad.h"
#include "renderer/passes/geometry_pass.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h" // for BloomDirection enum

namespace RealmEngine
{
    BloomPass::~BloomPass() = default;

    BloomPass::BloomPass(const std::string& shader_path,
                         bool               enabled,
                         float              intensity,
                         int                iterations,
                         BloomDirection     direction) :
        RenderPass("bloom"), m_shader_path(shader_path), m_enabled(enabled), m_intensity(intensity),
        m_iterations(iterations), m_direction(direction)
    {}

    void BloomPass::initialize(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/bloom.vert", m_shader_path + "/bloom.frag");
    }

    void BloomPass::execute(const RenderContext& ctx)
    {
        if (!m_enabled || !m_geometry_pass || !m_quad)
            return;

        auto* pbr_fb = m_geometry_pass->getFramebuffer();
        if (!pbr_fb)
            return;

        auto* bloom_tex = pbr_fb->getColorAttachment(1);
        if (!bloom_tex)
            return;

        glm::vec2 blur_direction_x = glm::vec2(1.0f, 0.0f);
        glm::vec2 blur_direction_y = glm::vec2(0.0f, 1.0f);

        switch (m_direction)
        {
            case BloomDirection::HORIZONTAL:
                blur_direction_y = blur_direction_x;
                break;
            case BloomDirection::VERTICAL:
                blur_direction_x = blur_direction_y;
                break;
            default:
                break;
        }

        bloom_tex->generateMipmaps();

        m_shader->use();

        static constexpr int BLOOM_MAX_MIP = 5;
        for (int mip = 0; mip <= BLOOM_MAX_MIP; ++mip)
        {
            m_framebuffers[0]->setMipLevel(mip);
            m_framebuffers[1]->setMipLevel(mip);

            // First iteration samples from main PBR bloom buffer
            m_framebuffers[0]->bind();
            ctx.device->bindTexture(0, *bloom_tex);
            m_shader->setInt("sampleMipLevel", mip);
            m_shader->setVec2("blurDirection", blur_direction_x);
            m_quad->draw();

            // Ping-pong
            uint32_t dst = 1;
            uint32_t src = 0;
            for (int i = 1; i < m_iterations; ++i)
            {
                src = (dst == 1) ? 0 : 1;
                m_framebuffers[dst]->bind();
                auto blur_dir = (dst == 1) ? blur_direction_y : blur_direction_x;
                m_shader->setVec2("blurDirection", blur_dir);
                ctx.device->bindTexture(0, *m_framebuffers[src]->getColorAttachment(0));
                m_quad->draw();
                dst = src;
            }
            m_result_idx = dst;
        }
    }

    void BloomPass::dispose()
    {
        m_shader.reset();
        m_framebuffers[0].reset();
        m_framebuffers[1].reset();
    }

    void BloomPass::setFramebuffers(std::unique_ptr<RHIFramebuffer> fb0, std::unique_ptr<RHIFramebuffer> fb1)
    {
        m_framebuffers[0] = std::move(fb0);
        m_framebuffers[1] = std::move(fb1);
    }

    RHITexture* BloomPass::getResultTexture() const
    {
        return m_framebuffers[m_result_idx] ? m_framebuffers[m_result_idx]->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
