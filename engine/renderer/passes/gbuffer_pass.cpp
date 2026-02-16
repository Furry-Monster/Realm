#include "renderer/passes/gbuffer_pass.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/render_camera.h"
#include "renderer/render_scene.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    GBufferPass::~GBufferPass() = default;

    GBufferPass::GBufferPass(const std::string& shader_path,
                             float              clear_r,
                             float              clear_g,
                             float              clear_b,
                             float              clear_a) :
        RenderPass("gbuffer"), m_shader_path(shader_path), m_clear_r(clear_r), m_clear_g(clear_g), m_clear_b(clear_b),
        m_clear_a(clear_a)
    {}

    void GBufferPass::init(RHIDevice& device)
    {
        m_shader =
            device.createShader(m_shader_path + "/deferred_gbuffer.vert", m_shader_path + "/deferred_gbuffer.frag");
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
        glm::mat4 projection = ctx.camera->getProjMatrix();
        glm::mat4 view       = ctx.camera->getViewMatrix();

        // drawOpaque already skips hair, transparent, and custom shader meshes
        for (size_t i = 0; i < ctx.scene->m_render_objects.size(); ++i)
        {
            auto&     ro    = ctx.scene->m_render_objects[i];
            glm::mat4 model = (i < ctx.scene->m_render_model_matrices.size()) ? ctx.scene->m_render_model_matrices[i] :
                                                                                glm::mat4(1.0f);
            m_shader->setMVP(model, view, projection);
            ro->drawOpaque(*m_shader);
        }
    }

    void GBufferPass::dispose()
    {
        m_framebuffer.reset();
        m_shader.reset();
    }

} // namespace RealmEngine
