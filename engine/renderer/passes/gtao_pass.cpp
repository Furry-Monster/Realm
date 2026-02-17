#include "renderer/passes/gtao_pass.h"

#include <cmath>
#include <cstddef>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
    GTAOPass::~GTAOPass() = default;

    GTAOPass::GTAOPass(const std::string& shader_path,
                       bool               enabled,
                       float              radius,
                       int                num_directions,
                       int                num_steps) :
        RenderPass("gtao"), m_shader_path(shader_path), m_enabled(enabled), m_radius(radius),
        m_num_directions(std::min(std::max(num_directions, 4), 8)),
        m_num_steps(std::min(std::max(num_steps, 4), 8))
    {
    }

    void GTAOPass::generateNoiseTexture(RHIDevice& device)
    {
        // 4x4 blue-noise rotation texture: each texel stores (cos(angle), sin(angle), jitter)
        constexpr int                         noise_size = 4;
        constexpr size_t                      texel_count = noise_size * noise_size;
        std::vector<glm::vec3>                noise(texel_count);
        std::uniform_real_distribution<float> jitter_dist(0.0f, 1.0f);
        std::default_random_engine            rng(42);

        for (size_t i = 0; i < texel_count; ++i)
        {
            // Spatially-distributed rotation angles via interleaved gradient
            float angle = glm::two_pi<float>() * (static_cast<float>(i) / static_cast<float>(texel_count))
                        + jitter_dist(rng) * glm::two_pi<float>() / static_cast<float>(texel_count);
            noise[i] = glm::vec3(std::cos(angle), std::sin(angle), jitter_dist(rng));
        }

        TextureDesc td;
        td.type       = TextureType::Texture2D;
        td.format     = TextureFormat::RGB16F;
        td.width      = noise_size;
        td.height     = noise_size;
        td.min_filter = TextureFilter::Nearest;
        td.mag_filter = TextureFilter::Nearest;
        td.wrap_s     = TextureWrap::Repeat;
        td.wrap_t     = TextureWrap::Repeat;
        td.data       = noise.data();

        m_noise_texture = device.createTexture(td);
    }

    void GTAOPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/gtao.vert", m_shader_path + "/builtin/gtao.frag");
        generateNoiseTexture(device);
    }

    void GTAOPass::execute(const RenderContext& ctx)
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

        ctx.device->bindTexture(TEXTURE_UNIT_GTAO_DEPTH, *depth_tex);
        m_shader->setInt("depthTexture", TEXTURE_UNIT_GTAO_DEPTH);
        ctx.device->bindTexture(TEXTURE_UNIT_GTAO_NOISE, *m_noise_texture);
        m_shader->setInt("noiseTexture", TEXTURE_UNIT_GTAO_NOISE);

        glm::mat4 proj     = ctx.camera->getProjMatrix();
        glm::mat4 inv_proj = glm::inverse(proj);
        m_shader->setMat4("projection", proj);
        m_shader->setMat4("invProjection", inv_proj);

        m_shader->setVec2("noiseScale",
                          glm::vec2(static_cast<float>(ctx.viewport_width) / 4.0f,
                                    static_cast<float>(ctx.viewport_height) / 4.0f));
        m_shader->setVec2("texelSize",
                          glm::vec2(1.0f / static_cast<float>(ctx.viewport_width),
                                    1.0f / static_cast<float>(ctx.viewport_height)));
        m_shader->setFloat("radius", m_radius);
        m_shader->setInt("numDirections", m_num_directions);
        m_shader->setInt("numSteps", m_num_steps);

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void GTAOPass::dispose()
    {
        m_framebuffer.reset();
        m_noise_texture.reset();
        m_shader.reset();
    }

    RHITexture* GTAOPass::getResultTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
