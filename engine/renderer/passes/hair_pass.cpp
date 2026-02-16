#include "renderer/passes/hair_pass.h"

#include <algorithm>

#include "renderer/light.h"
#include "renderer/passes/shadow_pass.h"
#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_buffer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    HairPass::~HairPass() = default;

    HairPass::HairPass(const std::string& shader_path) : RenderPass("hair"), m_shader_path(shader_path) {}

    void HairPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/hair.vert", m_shader_path + "/hair.frag");
        m_shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);
        m_light_ubo = device.createBuffer(BufferType::Uniform, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
    }

    void HairPass::execute(const RenderContext& ctx)
    {
        if (!m_scene_color || !m_scene_color->getFramebuffer())
            return;

        auto* fb = m_scene_color->getFramebuffer();
        fb->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setDepthTest(true);
        ctx.device->setDepthFunc(DepthFunc::Less);
        ctx.device->setBlend(true);
        ctx.device->setBlendFunc(
            BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
        ctx.device->setDepthWrite(false);

        m_shader->use();

        ctx.camera->update();
        glm::vec3 cam_pos    = ctx.camera->getPosition();
        glm::mat4 projection = ctx.camera->getProjMatrix();
        glm::mat4 view       = ctx.camera->getViewMatrix();
        m_shader->setVec3("cameraPosition", cam_pos);

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

        m_shader->setInt("displayMode", static_cast<int>(ctx.display_mode));

        if (m_ibl_diffuse)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP, *m_ibl_diffuse);
            m_shader->setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        }

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

        for (size_t i = 0; i < ctx.scene->m_render_objects.size(); ++i)
        {
            auto&     ro    = ctx.scene->m_render_objects[i];
            glm::mat4 model = (i < ctx.scene->m_render_model_matrices.size()) ? ctx.scene->m_render_model_matrices[i] :
                                                                                glm::mat4(1.0f);

            for (size_t m = 0; m < ro->getMeshCount(); ++m)
            {
                auto* mesh = ro->getMesh(m);
                if (!mesh->isHair())
                    continue;

                int   layers     = mesh->m_material.hair_layers;
                float layer_step = mesh->m_material.hair_layer_step;

                for (int layer = 0; layer < layers; ++layer)
                {
                    m_shader->setFloat("layerIndex", static_cast<float>(layer));
                    m_shader->setFloat("layerStep", layer_step);
                    m_shader->setMVP(model, view, projection);
                    mesh->drawHair(*m_shader);
                }
            }
        }

        ctx.device->setBlend(false);
        ctx.device->setDepthWrite(true);
    }

    void HairPass::dispose()
    {
        m_shader.reset();
        m_light_ubo.reset();
    }

    void HairPass::setIBLTextures(RHITexture* diffuse_irradiance) { m_ibl_diffuse = diffuse_irradiance; }

} // namespace RealmEngine
