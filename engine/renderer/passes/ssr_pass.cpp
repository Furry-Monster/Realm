#include "renderer/passes/ssr_pass.h"

#include <glm/gtc/matrix_inverse.hpp>

#include "renderer/fullscreen_quad.h"
#include "renderer/passes/gbuffer_pass.h"
#include "renderer/passes/hiz_pass.h"
#include "renderer/render_camera.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    static constexpr int TEX_UNIT_SCENE_COLOR    = 0;
    static constexpr int TEX_UNIT_HIZ_DEPTH      = 1;
    static constexpr int TEX_UNIT_NORMAL_METAL   = 2;
    static constexpr int TEX_UNIT_EMISSIVE_ROUGH = 3;
    static constexpr int TEX_UNIT_SCENE_DEPTH    = 4;

    SSRPass::~SSRPass() = default;

    SSRPass::SSRPass(const std::string& shader_path,
                     const bool         enabled,
                     const int          max_steps,
                     const float        max_distance) :
        RenderPass("ssr"), m_shader_path(shader_path), m_enabled(enabled), m_max_steps(max_steps),
        m_max_distance(max_distance)
    {}

    void SSRPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/ssr.vert", m_shader_path + "/builtin/ssr.frag");
    }

    void SSRPass::execute(const RenderContext& ctx)
    {
        if (!m_enabled || !m_scene_color || !m_hiz_pass || !m_quad || !m_framebuffer)
            return;

        auto* scene_fb = m_scene_color->getFramebuffer();
        if (!scene_fb)
            return;

        auto* scene_color_tex = scene_fb->getColorAttachment(0);
        auto* hiz_tex         = m_hiz_pass->getHiZTexture();
        if (!scene_color_tex || !hiz_tex)
            return;

        RHITexture* normal_metallic_tex = nullptr;
        RHITexture* emissive_rough_tex  = nullptr;
        RHITexture* scene_depth_tex     = nullptr;

        if (m_gbuffer_pass)
        {
            auto* gbuf = m_gbuffer_pass->getFramebuffer();
            if (gbuf)
            {
                normal_metallic_tex = gbuf->getColorAttachment(1); // worldNormal.xyz, metallic
                emissive_rough_tex  = gbuf->getColorAttachment(2); // emissive.rgb, roughness
                scene_depth_tex     = gbuf->getDepthAttachment();
            }
        }

        if (!scene_depth_tex)
            scene_depth_tex = scene_fb->getDepthAttachment();

        if (!normal_metallic_tex || !scene_depth_tex)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ctx.device->clear(ClearFlags::Color);
        ctx.device->setDepthTest(false);

        m_shader->use();

        ctx.device->bindTexture(TEX_UNIT_SCENE_COLOR, *scene_color_tex);
        m_shader->setInt("uSceneColor", TEX_UNIT_SCENE_COLOR);

        ctx.device->bindTexture(TEX_UNIT_HIZ_DEPTH, *hiz_tex);
        m_shader->setInt("uHiZDepth", TEX_UNIT_HIZ_DEPTH);

        ctx.device->bindTexture(TEX_UNIT_NORMAL_METAL, *normal_metallic_tex);
        m_shader->setInt("uNormalMetallic", TEX_UNIT_NORMAL_METAL);

        if (emissive_rough_tex)
        {
            ctx.device->bindTexture(TEX_UNIT_EMISSIVE_ROUGH, *emissive_rough_tex);
            m_shader->setInt("uEmissiveRoughness", TEX_UNIT_EMISSIVE_ROUGH);
        }

        ctx.device->bindTexture(TEX_UNIT_SCENE_DEPTH, *scene_depth_tex);
        m_shader->setInt("uSceneDepth", TEX_UNIT_SCENE_DEPTH);

        const glm::mat4 view       = ctx.camera->getViewMatrix();
        const glm::mat4 projection = ctx.camera->getProjMatrix();
        const glm::mat4 inv_view   = glm::inverse(view);
        const glm::mat4 inv_proj   = glm::inverse(projection);

        m_shader->setMat4("uView", view);
        m_shader->setMat4("uProjection", projection);
        m_shader->setMat4("uInvView", inv_view);
        m_shader->setMat4("uInvProjection", inv_proj);

        m_shader->setInt("uMaxSteps", m_max_steps);
        m_shader->setFloat("uMaxDistance", m_max_distance);
        m_shader->setFloat("uNearPlane", ctx.camera->getNearPlane());
        m_shader->setFloat("uFarPlane", ctx.camera->getFarPlane());
        m_shader->setVec2("uScreenSize",
                          glm::vec2(static_cast<float>(ctx.viewport_width), static_cast<float>(ctx.viewport_height)));

        const int hiz_mip_count =
            1 + static_cast<int>(
                    std::floor(std::log2(static_cast<double>(std::max(hiz_tex->getWidth(), hiz_tex->getHeight())))));
        m_shader->setInt("uHiZMipCount", hiz_mip_count);

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void SSRPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

    RHITexture* SSRPass::getResultTexture() const
    {
        return m_framebuffer ? m_framebuffer->getColorAttachment(0) : nullptr;
    }

} // namespace RealmEngine
