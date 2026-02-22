#include "functional/render/passes/gbuffer_pass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

#include "functional/render/material.h"
#include "functional/render/render_camera.h"
#include "functional/render/render_mesh.h"
#include "functional/render/render_object.h"
#include "functional/render/render_scene.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_framebuffer.h"
#include "functional/render/rhi/rhi_shader.h"
#include "functional/render/rhi/rhi_texture.h"
#include "functional/render/rhi/rhi_types.h"

namespace RealmEngine
{
    // Maps ShadingModel to G-Buffer shadingModelID (RT0.a)
    static int shadingModelToID(const ShadingModel model)
    {
        switch (model)
        {
            case ShadingModel::StandardPBR:
                return 0;
            default:
                return 0;
        }
    }

    GBufferPass::~GBufferPass() noexcept = default;

    GBufferPass::GBufferPass(const std::string& shader_path,
                             const float        clear_r,
                             const float        clear_g,
                             const float        clear_b,
                             const float        clear_a) :
        RenderPass("gbuffer"), m_shader_path(shader_path), m_clear_r(clear_r), m_clear_g(clear_g), m_clear_b(clear_b),
        m_clear_a(clear_a)
    {}

    void GBufferPass::init(RHIDevice& device)
    {
        m_shader = device.createShader(m_shader_path + "/builtin/deferred_gbuffer.vert",
                                       m_shader_path + "/builtin/deferred_gbuffer.frag");

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

    void GBufferPass::execute(const RenderContext& ctx)
    {
        if (!m_framebuffer)
            return;

        m_framebuffer->bind();
        ctx.device->setViewport(0, 0, ctx.viewport_width, ctx.viewport_height);
        ctx.device->setClearColor(m_clear_r, m_clear_g, m_clear_b, m_clear_a);
        ctx.device->clear(ClearFlags::Color | ClearFlags::Depth);
        ctx.device->setDepthTest(true);
        ctx.device->setDepthFunc(DepthFunc::Less);
        ctx.device->setBlend(false);
        ctx.device->setDepthWrite(true);

        m_shader->use();

        ctx.camera->update();
        const glm::mat4 projection = ctx.camera->getProjMatrix();
        const glm::mat4 view       = ctx.camera->getViewMatrix();

        const auto& objects  = ctx.scene->getRenderObjects();
        const auto& matrices = ctx.scene->getRenderModelMatrices();
        for (size_t i = 0; i < objects.size(); ++i)
        {
            auto&     ro    = *objects[i];
            glm::mat4 model = (i < matrices.size()) ? matrices[i] : glm::mat4(1.0f);
            m_shader->setMVP(model, view, projection);

            ro.forEachMesh([&](RenderMesh& mesh) {
                if (!mesh.m_material.isDeferred())
                    return;

                for (int u = TEXTURE_UNIT_ALBEDO; u <= TEXTURE_UNIT_OPACITY; ++u)
                    ctx.device->bindTexture(u, *m_default_white);

                const int model_id = shadingModelToID(mesh.m_material.shading_model);
                m_shader->setInt("shadingModelID", model_id);

                ctx.device->setCullFace(mesh.m_material.isDoubleSided() ? CullFace::None : CullFace::Back);
                mesh.draw(*m_shader);
            });
        }
    }

    void GBufferPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
        m_default_white.reset();
    }

} // namespace RealmEngine
