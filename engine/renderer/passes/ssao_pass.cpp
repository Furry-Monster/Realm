#include "renderer/passes/ssao_pass.h"

#include <random>

#include "renderer/fullscreen_quad.h"
#include "renderer/material.h"
#include "renderer/render_camera.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    SSAOPass::~SSAOPass() = default;

    SSAOPass::SSAOPass(const std::string& shader_path,
                       bool               enabled,
                       float              radius,
                       float              bias,
                       int                kernel_size,
                       int                noise_size) :
        RenderPass("ssao"), m_shader_path(shader_path), m_enabled(enabled), m_radius(radius), m_bias(bias),
        m_kernel_size(std::min(std::max(kernel_size, 1), 64)), m_noise_size(std::max(noise_size, 2))
    {
        generateKernel();
    }

    void SSAOPass::generateKernel()
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::default_random_engine            rng;

        m_kernel.resize(m_kernel_size);
        for (int i = 0; i < m_kernel_size; ++i)
        {
            glm::vec3 sample(dist(rng) * 2.0f - 1.0f, dist(rng) * 2.0f - 1.0f, dist(rng));
            sample = glm::normalize(sample);
            sample *= dist(rng);

            float scale = static_cast<float>(i) / static_cast<float>(m_kernel_size);
            scale       = 0.1f + 0.9f * scale * scale;
            sample *= scale;
            m_kernel[i] = sample;
        }
    }

    void SSAOPass::generateNoiseTexture(RHIDevice& device)
    {
        std::vector<glm::vec3>                noise(m_noise_size * m_noise_size);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        std::default_random_engine            rng;

        for (int i = 0; i < m_noise_size * m_noise_size; ++i)
            noise[i] = glm::vec3(dist(rng), dist(rng), 0.0f);

        TextureDesc td;
        td.type   = TextureType::Texture2D;
        td.format = TextureFormat::RGB16F;
        td.width  = m_noise_size;
        td.height = m_noise_size;
        td.wrap_s = TextureWrap::Repeat;
        td.wrap_t = TextureWrap::Repeat;
        td.data   = noise.data();

        m_noise_texture = device.createTexture(td);
    }

    void SSAOPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/ssao.vert", m_shader_path + "/ssao.frag");
        generateNoiseTexture(device);
    }

    void SSAOPass::execute(const RenderContext& ctx)
    {
        if (!m_enabled || !m_scene_color || !m_quad || !m_framebuffer)
            return;

        auto* geo_fb = m_scene_color->getFramebuffer();
        if (!geo_fb)
            return;

        RHITexture* depth_tex = geo_fb->getDepthAttachment();
        if (!depth_tex)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setDepthTest(false);
        ctx.device->clear(ClearFlags::Color);

        m_shader->use();

        ctx.device->bindTexture(TEXTURE_UNIT_SSAO_DEPTH, *depth_tex);
        m_shader->setInt("depthTexture", TEXTURE_UNIT_SSAO_DEPTH);
        ctx.device->bindTexture(TEXTURE_UNIT_SSAO_NOISE, *m_noise_texture);
        m_shader->setInt("noiseTexture", TEXTURE_UNIT_SSAO_NOISE);

        glm::mat4 proj     = ctx.camera->getProjMatrix();
        glm::mat4 inv_proj = glm::inverse(proj);
        m_shader->setMat4("projection", proj);
        m_shader->setMat4("invProjection", inv_proj);

        m_shader->setVec2("noiseScale",
                          glm::vec2(static_cast<float>(ctx.viewport_width) / static_cast<float>(m_noise_size),
                                    static_cast<float>(ctx.viewport_height) / static_cast<float>(m_noise_size)));
        m_shader->setFloat("radius", m_radius);
        m_shader->setFloat("bias", m_bias);
        m_shader->setInt("kernelSize", m_kernel_size);
        m_shader->setVec3Array("samples", m_kernel);

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void SSAOPass::dispose()
    {
        m_framebuffer.reset();
        m_noise_texture.reset();
        m_shader.reset();
    }

    RHITexture* SSAOPass::getResultTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
