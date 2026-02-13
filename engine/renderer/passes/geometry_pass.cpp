#include "renderer/passes/geometry_pass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <cstring>

#include "renderer/light.h"
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
    GeometryPass::~GeometryPass() = default;

    GeometryPass::GeometryPass(const std::string& shader_path,
                               float              clear_r,
                               float              clear_g,
                               float              clear_b,
                               float              clear_a,
                               float              bloom_brightness_cutoff) :
        RenderPass("geometry"), m_shader_path(shader_path), m_clear_r(clear_r), m_clear_g(clear_g), m_clear_b(clear_b),
        m_clear_a(clear_a), m_bloom_brightness_cutoff(bloom_brightness_cutoff)
    {}

    void GeometryPass::initialize(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/pbr.vert", m_shader_path + "/pbr.frag");
        m_shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);

        // Light UBO
        m_light_ubo = device.createBuffer(BufferType::Uniform, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
    }

    void GeometryPass::execute(const RenderContext& ctx)
    {
        if (!m_framebuffer)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setClearColor(m_clear_r, m_clear_g, m_clear_b, m_clear_a);
        ctx.device->clear(ClearFlags::Color | ClearFlags::Depth);
        ctx.device->setDepthTest(true);
        ctx.device->setDepthFunc(DepthFunc::Less);

        m_shader->use();

        // Camera
        ctx.camera->update();
        glm::vec3 cam_pos    = ctx.camera->getPosition();
        glm::mat4 projection = ctx.camera->getProjMatrix();
        glm::mat4 view       = ctx.camera->getViewMatrix();
        m_shader->setVec3("cameraPosition", cam_pos);

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
            ctx.device->bindTexture(TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP, *m_ibl_diffuse);
            m_shader->setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        }
        if (m_ibl_prefiltered)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_PREFILTERED_ENV_MAP, *m_ibl_prefiltered);
            m_shader->setInt("prefilteredEnvMap", TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        }
        if (m_ibl_brdf)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_BRDF_CONVOLUTION_MAP, *m_ibl_brdf);
            m_shader->setInt("brdfConvolutionMap", TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        }

        m_shader->setFloat("bloomBrightnessCutoff", m_bloom_brightness_cutoff);

        // Shadow
        if (m_shadow_pass && m_shadow_pass->isShadowEnabled())
        {
            auto* depth = m_shadow_pass->getFramebuffer()->getDepthAttachment();
            if (depth)
            {
                ctx.device->bindTexture(TEXTURE_UNIT_SHADOW_MAP, *depth);
                m_shader->setInt("shadowMap", TEXTURE_UNIT_SHADOW_MAP);
            }
            m_shader->setMat4("lightSpaceMatrix", m_shadow_pass->getLightSpaceMatrix());
            m_shader->setBool("shadowEnabled", true);
        }
        else
        {
            m_shader->setMat4("lightSpaceMatrix", glm::mat4(1.0f));
            m_shader->setBool("shadowEnabled", false);
        }

        // Draw objects
        for (auto& ro : ctx.scene->m_render_objects)
        {
            glm::mat4 model = glm::toMat4(ro->getOrientation());
            model           = glm::translate(model, ro->getPosition());
            model           = glm::scale(model, ro->getScale());

            m_shader->setMVP(model, view, projection);
            ctx.device->setDepthWrite(true);
            ro->draw(*m_shader);
        }
    }

    void GeometryPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
        m_light_ubo.reset();
    }

    void GeometryPass::setIBLTextures(RHITexture* diffuse_irradiance, RHITexture* prefiltered_env, RHITexture* brdf_lut)
    {
        m_ibl_diffuse     = diffuse_irradiance;
        m_ibl_prefiltered = prefiltered_env;
        m_ibl_brdf        = brdf_lut;
    }

} // namespace RealmEngine
