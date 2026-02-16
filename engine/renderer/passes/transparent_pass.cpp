#include "renderer/passes/transparent_pass.h"

#include <algorithm>

#include "renderer/light.h"
#include "renderer/material.h"
#include "renderer/passes/shadow_pass.h"
#include "renderer/render_camera.h"
#include "renderer/render_mesh.h"
#include "renderer/render_object.h"
#include "renderer/render_scene.h"
#include "renderer/scene_color_source.h"
#include "rhi/rhi_buffer.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    TransparentPass::TransparentPass(const std::string& shader_path) :
        RenderPass("transparent"), m_shader_path(shader_path)
    {}

    TransparentPass::~TransparentPass() = default;

    void TransparentPass::init(RHIDevice& device)
    {
        m_pbr_shader = device.createShader(m_shader_path + "/pbr.vert", m_shader_path + "/pbr.frag");
        m_pbr_shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);
        m_light_ubo = device.createBuffer(BufferType::Uniform, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
    }

    RHIShader* TransparentPass::resolveShader(const Material& mat, RHIDevice& device)
    {
        if (mat.hasCustomShader())
            return m_shader_cache.getOrCreate(mat.vert_path, mat.frag_path, device);
        return m_pbr_shader.get();
    }

    void TransparentPass::setupEngineUniforms(RHIShader& shader, const RenderContext& ctx)
    {
        shader.setVec3("cameraPosition", ctx.camera->getPosition());
        shader.setInt("displayMode", static_cast<int>(ctx.display_mode));

        if (m_ibl_diffuse)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP, *m_ibl_diffuse);
            shader.setInt("diffuseIrradianceMap", TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP);
        }
        if (m_ibl_prefiltered)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_PREFILTERED_ENV_MAP, *m_ibl_prefiltered);
            shader.setInt("prefilteredEnvMap", TEXTURE_UNIT_PREFILTERED_ENV_MAP);
        }
        if (m_ibl_brdf)
        {
            ctx.device->bindTexture(TEXTURE_UNIT_BRDF_CONVOLUTION_MAP, *m_ibl_brdf);
            shader.setInt("brdfConvolutionMap", TEXTURE_UNIT_BRDF_CONVOLUTION_MAP);
        }

        if (m_shadow_pass && m_shadow_pass->isShadowEnabled())
        {
            auto* depth = m_shadow_pass->getFramebuffer()->getDepthAttachment();
            if (depth)
            {
                ctx.device->bindTexture(TEXTURE_UNIT_SHADOW_MAP, *depth);
                shader.setInt("shadowMap", TEXTURE_UNIT_SHADOW_MAP);
            }
            shader.setMat4("lightSpaceMatrix", m_shadow_pass->getLightSpaceMatrix());
            shader.setBool("shadowEnabled", true);
        }
        else
        {
            shader.setMat4("lightSpaceMatrix", glm::mat4(1.0f));
            shader.setBool("shadowEnabled", false);
        }
    }

    void TransparentPass::execute(const RenderContext& ctx)
    {
        if (!m_scene_color || !m_scene_color->getFramebuffer())
            return;

        auto* fb = m_scene_color->getFramebuffer();
        fb->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setDepthTest(true);
        ctx.device->setDepthFunc(DepthFunc::Less);

        ctx.camera->update();
        glm::vec3 cam_pos    = ctx.camera->getPosition();
        glm::mat4 projection = ctx.camera->getProjMatrix();
        glm::mat4 view       = ctx.camera->getViewMatrix();

        // Upload light data
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

        // Collect transparent draw commands sorted by distance (back-to-front)
        struct DrawCmd
        {
            RenderMesh* mesh;
            glm::mat4   model;
            RHIShader*  shader;
            float       distance;
        };
        std::vector<DrawCmd> commands;

        for (size_t i = 0; i < ctx.scene->m_render_objects.size(); ++i)
        {
            auto& ro = ctx.scene->m_render_objects[i];
            if (!ro)
                continue;

            glm::mat4 model = (i < ctx.scene->m_render_model_matrices.size()) ? ctx.scene->m_render_model_matrices[i] :
                                                                                glm::mat4(1.0f);
            glm::vec3 obj_pos(model[3]);
            float     dist = glm::length(cam_pos - obj_pos);

            ro->forEachMesh([&](RenderMesh& mesh) {
                if (!mesh.m_material.isTransparent())
                    return;

                RHIShader* shader = resolveShader(mesh.m_material, *ctx.device);
                if (!shader)
                    return;

                commands.push_back({&mesh, model, shader, dist});
            });
        }

        // Sort back-to-front
        std::sort(commands.begin(), commands.end(), [](const DrawCmd& a, const DrawCmd& b) {
            return a.distance > b.distance;
        });

        // Render
        ctx.device->setBlend(true);
        ctx.device->setBlendFunc(
            BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
        ctx.device->setDepthWrite(false);

        RHIShader* active_shader = nullptr;

        for (const auto& cmd : commands)
        {
            if (cmd.shader != active_shader)
            {
                cmd.shader->use();
                cmd.shader->bindUniformBlock("LightBlock", LIGHT_UBO_BINDING_POINT);
                setupEngineUniforms(*cmd.shader, ctx);
                cmd.shader->setBool("isTransparentPass", true);
                active_shader = cmd.shader;
            }

            ctx.device->setCullFace(cmd.mesh->m_material.isDoubleSided() ? CullFace::None : CullFace::Back);
            cmd.shader->setMVP(cmd.model, view, projection);
            cmd.mesh->draw(*cmd.shader);
        }

        ctx.device->setBlend(false);
        ctx.device->setDepthWrite(true);
    }

    void TransparentPass::dispose()
    {
        m_pbr_shader.reset();
        m_light_ubo.reset();
        m_shader_cache.clear();
    }

    void TransparentPass::setIBLTextures(RHITexture* diffuse, RHITexture* prefiltered, RHITexture* brdf)
    {
        m_ibl_diffuse     = diffuse;
        m_ibl_prefiltered = prefiltered;
        m_ibl_brdf        = brdf;
    }

} // namespace RealmEngine
