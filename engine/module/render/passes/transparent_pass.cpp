#include "module/render/passes/transparent_pass.h"

#include <algorithm>
#include <cstddef>

#include "module/render/light.h"
#include "module/render/light_probe_data.h"
#include "module/render/material.h"
#include "module/render/passes/csm_shadow_pass.h"
#include "module/render/render_camera.h"
#include "module/render/render_mesh.h"
#include "module/render/render_object.h"
#include "module/render/render_scene.h"
#include "module/render/rhi/rhi_buffer.h"
#include "module/render/rhi/rhi_device.h"
#include "module/render/rhi/rhi_framebuffer.h"
#include "module/render/rhi/rhi_shader.h"
#include "module/render/rhi/rhi_texture.h"
#include "module/render/rhi/rhi_types.h"
#include "module/render/scene_color_source.h"
#include "functional/scene/scene.h"

namespace RealmEngine
{
    TransparentPass::TransparentPass(const std::string& shader_path) :
        RenderPass("transparent"), m_shader_path(shader_path)
    {}

    TransparentPass::~TransparentPass() noexcept = default;

    static constexpr size_t PROBE_SSBO_SIZE = 16 + LightProbeGPUData::MAX_ACTIVE_PROBES * 160;

    void TransparentPass::init(RHIDevice& device)
    {
        m_pbr_shader = device.createShader(m_shader_path + "/builtin/pbr.vert", m_shader_path + "/builtin/pbr.frag");
        m_pbr_shader->bindShaderStorageBlock("LightBuffer", 1);
        m_pbr_shader->bindShaderStorageBlock("ProbeBuffer", 5);
        m_light_ssbo = device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, BUFFER_SIZE);
        m_probe_ssbo = device.createBuffer(BufferType::ShaderStorage, BufferUsage::Dynamic, nullptr, PROBE_SSBO_SIZE);

        constexpr uint8_t white[] = {255, 255, 255, 255};
        TextureDesc       td;
        td.type         = TextureType::Texture2D;
        td.format       = TextureFormat::RGBA8;
        td.width        = 1;
        td.height       = 1;
        td.data         = white;
        td.min_filter   = TextureFilter::Nearest;
        td.mag_filter   = TextureFilter::Nearest;
        m_default_white = device.createTexture(td);
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
                shader.setInt("shadowMapArray", TEXTURE_UNIT_SHADOW_MAP);
            }
            const auto& cascades = m_shadow_pass->getCascades();
            shader.setInt("cascadeCount", CSMShadowPass::CASCADE_COUNT);
            std::vector<float> splits(CSMShadowPass::CASCADE_COUNT);
            for (size_t c = 0; c < CSMShadowPass::CASCADE_COUNT; ++c)
            {
                shader.setMat4("cascadeVP[" + std::to_string(c) + "]", cascades[c].light_view_proj);
                splits[c] = cascades[c].split_depth;
            }
            shader.setFloatArray("cascadeSplits", splits);
            shader.setFloat("lightSize", m_shadow_pass->getLightSize());
            shader.setBool("shadowEnabled", true);
        }
        else
        {
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
        const glm::vec3 cam_pos    = ctx.camera->getPosition();
        const glm::mat4 projection = ctx.camera->getProjMatrix();
        const glm::mat4 view       = ctx.camera->getViewMatrix();

        // Upload light data
        {
            const size_t count   = std::min(ctx.scene->getLights().size(), MAX_LIGHTS);
            const int    count_i = static_cast<int>(count);
            LightData    data[MAX_LIGHTS] {};
            for (size_t i = 0; i < count; ++i)
            {
                auto& l             = ctx.scene->getLights()[i];
                data[i].position    = glm::vec4(l.position, static_cast<float>(static_cast<int>(l.type)));
                data[i].direction   = glm::vec4(l.direction, l.intensity);
                data[i].color       = glm::vec4(l.color, l.constant);
                data[i].attenuation = glm::vec4(l.linear, l.quadratic, l.range, l.inner_cone_angle);
                data[i].spot_area   = glm::vec4(l.outer_cone_angle, l.width, l.height, 0.0f);
            }
            m_light_ssbo->setSubData(&count_i, 0, sizeof(int));
            m_light_ssbo->setSubData(data, 16, count * sizeof(LightData));
            m_light_ssbo->bindBase(1);
        }

        // Upload probe data
        bool   probes_active = false;
        Scene* ecs_scene     = ctx.scene ? ctx.scene->getScene() : nullptr;
        if (ecs_scene && m_probe_ssbo)
        {
            LightProbeGPUData probe_data;
            probe_data.collectFromScene(*ecs_scene);

            if (probe_data.probe_count > 0)
            {
                m_probe_ssbo->setSubData(&probe_data.probe_count, 0, sizeof(int));
                m_probe_ssbo->setSubData(
                    probe_data.probes.data(), 16, probe_data.probes.size() * sizeof(LightProbeGPUData::ProbeInfo));
                probes_active = true;
            }
            else
            {
                constexpr int zero = 0;
                m_probe_ssbo->setSubData(&zero, 0, sizeof(int));
            }
            m_probe_ssbo->bindBase(5);
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

        const auto& objects  = ctx.scene->getRenderObjects();
        const auto& matrices = ctx.scene->getRenderModelMatrices();
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (!objects[i])
                continue;

            auto&     ro    = *objects[i];
            glm::mat4 model = (i < matrices.size()) ? matrices[i] : glm::mat4(1.0f);
            glm::vec3 obj_pos(model[3]);
            float     dist = glm::length(cam_pos - obj_pos);

            ro.forEachMesh([&](RenderMesh& mesh) {
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

        const RHIShader* active_shader = nullptr;

        for (const auto& cmd : commands)
        {
            if (cmd.shader != active_shader)
            {
                cmd.shader->use();
                cmd.shader->bindShaderStorageBlock("LightBuffer", 1);
                cmd.shader->bindShaderStorageBlock("ProbeBuffer", 5);
                setupEngineUniforms(*cmd.shader, ctx);
                cmd.shader->setBool("isTransparentPass", true);
                cmd.shader->setBool("probesEnabled", probes_active);
                active_shader = cmd.shader;
            }

            for (int u = TEXTURE_UNIT_ALBEDO; u <= TEXTURE_UNIT_OPACITY; ++u)
                ctx.device->bindTexture(u, *m_default_white);

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
        m_light_ssbo.reset();
        m_probe_ssbo.reset();
        m_default_white.reset();
        m_shader_cache.clear();
    }

    void TransparentPass::setIBLTextures(RHITexture* diffuse, RHITexture* prefiltered, RHITexture* brdf)
    {
        m_ibl_diffuse     = diffuse;
        m_ibl_prefiltered = prefiltered;
        m_ibl_brdf        = brdf;
    }

} // namespace RealmEngine
