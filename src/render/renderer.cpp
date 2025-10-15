#include "renderer.h"

#include "render/render_camera.h"
#include "render/render_pipeline.h"
#include "render/render_resource.h"
#include "render/render_swap_buffer.h"
#include "render/rhi.h"
#include "utils.h"

#include <memory>
#include <string>

namespace RealmEngine
{
    void Renderer::initialize()
    {
        m_rhi             = std::make_shared<RHI>();
        m_render_res      = std::make_shared<RenderResource>();
        m_render_swap_buf = std::make_shared<RenderSwapBuffer>();
        m_render_scene    = std::make_shared<RenderScene>();
        m_render_camera   = std::make_shared<RenderCamera>();

        m_rhi->initialize();
        m_render_res->initialize();
        m_render_swap_buf->initialize();
        m_render_camera->initialize();

        info("Renderer initalized.");
    }

    void Renderer::disposal()
    {
        m_render_camera->disposal();
        m_render_res->disposal();
        m_render_res->disposal();
        m_rhi->disposal();

        m_render_camera.reset();
        m_render_scene.reset();
        m_render_pipeline.reset();
        m_render_swap_buf.reset();
        m_render_res.reset();
        m_rhi.reset();

        info("Renderer disposed all resources.");
    }

    void Renderer::renderFrame()
    {
        m_render_swap_buf->swapData();
        processSwapData();

        m_render_scene->sortTransparentObjects(m_render_camera->getPosition());
        m_render_scene->cullObjects(m_render_camera->getFrustum());

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_render_pipeline->render();
    }

    void Renderer::setPipeline(PipelineType type)
    {
        std::string type_str;

        switch (type)
        {
            case PipelineType::Forward:
                m_render_pipeline.reset();
                m_render_pipeline = std::make_unique<ForwardPipeline>();

                // Set render context for ForwardPipeline
                if (auto* forward_pipeline = dynamic_cast<ForwardPipeline*>(m_render_pipeline.get()))
                {
                    forward_pipeline->setRenderContext(m_rhi, m_render_scene, m_render_camera);
                }

                // Initialize the pipeline
                m_render_pipeline->initialize();
                type_str = "Forward Rendering Pipeline";
                break;
            case PipelineType::Deffered:
                m_render_pipeline.reset();
                m_render_pipeline = std::make_unique<DefferedPipeline>();
                m_render_pipeline->initialize();
                type_str = "Deffered Rendering Pipeline";
                break;
            default:
                err("Unknown pipeline type.");
                break;
        }

        if (!type_str.empty())
            info("Pipeline switched to <" + type_str + ">.");
    }

    void Renderer::processSwapData()
    {
        RenderSwapData& swap_data = m_render_swap_buf->getRenderData();

        if (swap_data.dirty_camera.has_value())
        {
        }

        if (swap_data.dirty_lighting.has_value())
        {
        }

        if (swap_data.dirty_objects.has_value())
        {
        }

        if (swap_data.removed_objects.has_value())
        {
        }
    }

} // namespace RealmEngine