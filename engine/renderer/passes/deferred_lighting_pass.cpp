#include "renderer/passes/deferred_lighting_pass.h"

#include <algorithm>
#include <cstring>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/fullscreen_quad.h"
#include "renderer/light.h"
#include "renderer/passes/gbuffer_pass.h"
#include "renderer/passes/shadow_pass.h"
#include "renderer/render_camera.h"
#include "renderer/render_material.h"
#include "renderer/render_scene.h"
#include "rhi/rhi_buffer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    static constexpr int TEX_UNIT_G_ALBEDO_AO       = 0;
    static constexpr int TEX_UNIT_G_NORMAL_METALLIC = 1;
    static constexpr int TEX_UNIT_G_EMISSIVE_ROUGH  = 2;
    static constexpr int TEX_UNIT_G_DEPTH           = 3;
    static constexpr int TEX_UNIT_SHADOW            = 4;
    static constexpr int TEX_UNIT_IBL_DIFFUSE       = 5;
    static constexpr int TEX_UNIT_IBL_PREFILTERED   = 6;
    static constexpr int TEX_UNIT_IBL_BRDF          = 7;

    DeferredLightingPass::~DeferredLightingPass() = default;

    DeferredLightingPass::DeferredLightingPass(const std::string& shader_path) :
        RenderPass("deferred_lighting"), m_shader_path(shader_path)
    {}

    void DeferredLightingPass::init(RHIDevice& device)
    {
        m_shader =
            device.createShader(m_shader_path + "/deferred_lighting.vert", m_shader_path + "/deferred_lighting.frag");
        m_shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);
        m_light_ubo = device.createBuffer(BufferType::Uniform, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
    }

    void DeferredLightingPass::execute(const RenderContext& ctx)
    {
        if (!m_framebuffer || !m_gbuffer_pass || !m_quad)
            return;

        auto* gbuf = m_gbuffer_pass->getFramebuffer();
        if (!gbuf)
            return;

        // Blit G-Buffer depth into the lighting framebuffer so downstream passes
        // (SSAO, SkyboxPass depth-test, etc.) can read scene depth from it.
        ctx.device->blitFramebuffer(gbuf,
                                    m_framebuffer.get(),
                                    0,
                                    0,
                                    gbuf->getWidth(),
                                    gbuf->getHeight(),
                                    0,
                                    0,
                                    m_framebuffer->getWidth(),
                                    m_framebuffer->getHeight(),
                                    BlitMask::Depth);

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ctx.device->clear(ClearFlags::Color);
        ctx.device->setDepthTest(false);

        m_shader->use();

        // G-Buffer textures
        auto* albedo_ao   = gbuf->getColorAttachment(0);
        auto* normal_met  = gbuf->getColorAttachment(1);
        auto* emiss_rough = gbuf->getColorAttachment(2);
        auto* depth_tex   = gbuf->getDepthAttachment();

        if (!albedo_ao || !normal_met || !emiss_rough || !depth_tex)
            return;

        ctx.device->bindTexture(TEX_UNIT_G_ALBEDO_AO, *albedo_ao);
        m_shader->setInt("gAlbedoAO", TEX_UNIT_G_ALBEDO_AO);

        ctx.device->bindTexture(TEX_UNIT_G_NORMAL_METALLIC, *normal_met);
        m_shader->setInt("gNormalMetallic", TEX_UNIT_G_NORMAL_METALLIC);

        ctx.device->bindTexture(TEX_UNIT_G_EMISSIVE_ROUGH, *emiss_rough);
        m_shader->setInt("gEmissiveRoughness", TEX_UNIT_G_EMISSIVE_ROUGH);

        ctx.device->bindTexture(TEX_UNIT_G_DEPTH, *depth_tex);
        m_shader->setInt("gDepth", TEX_UNIT_G_DEPTH);

        // Camera
        ctx.camera->update();
        glm::vec3 cam_pos    = ctx.camera->getPosition();
        glm::mat4 projection = ctx.camera->getProjMatrix();
        glm::mat4 view       = ctx.camera->getViewMatrix();

        m_shader->setVec3("cameraPosition", cam_pos);
        m_shader->setMat4("invView", glm::inverse(view));
        m_shader->setMat4("invProjection", glm::inverse(projection));

        // Lights UBO
        {
            size_t    count   = std::min(ctx.scene->m_lights.size(), MAX_LIGHTS);
            int       count_i = static_cast<int>(count);
            LightData data[MAX_LIGHTS] {};
            for (size_t i = 0; i < count; ++i)
            {
                auto& l             = ctx.scene->m_lights[i];
                data[i].position    = glm::vec4(l.position, static_cast<float>(static_cast<int>(l.type)));
                data[i].direction   = glm::vec4(l.direction, l.intensity);
                data[i].color       = glm::vec4(l.color, l.constant);
                data[i].attenuation = glm::vec4(l.linear, l.quadratic, l.range, l.inner_cone_angle);
                data[i].spot_area   = glm::vec4(l.outer_cone_angle, l.width, l.height, 0.0f);
            }
            m_light_ubo->setSubData(&count_i, 0, sizeof(int));
            m_light_ubo->setSubData(data, 16, MAX_LIGHTS * sizeof(LightData));
            m_light_ubo->bindBase(LIGHT_UBO_BINDING_POINT);
        }

        // IBL textures
        if (m_ibl_diffuse)
        {
            ctx.device->bindTexture(TEX_UNIT_IBL_DIFFUSE, *m_ibl_diffuse);
            m_shader->setInt("diffuseIrradianceMap", TEX_UNIT_IBL_DIFFUSE);
        }
        if (m_ibl_prefiltered)
        {
            ctx.device->bindTexture(TEX_UNIT_IBL_PREFILTERED, *m_ibl_prefiltered);
            m_shader->setInt("prefilteredEnvMap", TEX_UNIT_IBL_PREFILTERED);
        }
        if (m_ibl_brdf)
        {
            ctx.device->bindTexture(TEX_UNIT_IBL_BRDF, *m_ibl_brdf);
            m_shader->setInt("brdfConvolutionMap", TEX_UNIT_IBL_BRDF);
        }

        // Shadow
        if (m_shadow_pass && m_shadow_pass->isShadowEnabled())
        {
            auto* shadow_depth = m_shadow_pass->getFramebuffer()->getDepthAttachment();
            if (shadow_depth)
            {
                ctx.device->bindTexture(TEX_UNIT_SHADOW, *shadow_depth);
                m_shader->setInt("shadowMap", TEX_UNIT_SHADOW);
            }
            m_shader->setMat4("lightSpaceMatrix", m_shadow_pass->getLightSpaceMatrix());
            m_shader->setBool("shadowEnabled", true);
        }
        else
        {
            m_shader->setMat4("lightSpaceMatrix", glm::mat4(1.0f));
            m_shader->setBool("shadowEnabled", false);
        }

        m_shader->setInt("displayMode", static_cast<int>(ctx.display_mode));

        m_quad->draw();

        ctx.device->setDepthTest(true);
    }

    void DeferredLightingPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
        m_light_ubo.reset();
    }

    void DeferredLightingPass::setIBLTextures(RHITexture* diffuse_irradiance,
                                              RHITexture* prefiltered_env,
                                              RHITexture* brdf_lut)
    {
        m_ibl_diffuse     = diffuse_irradiance;
        m_ibl_prefiltered = prefiltered_env;
        m_ibl_brdf        = brdf_lut;
    }

} // namespace RealmEngine
