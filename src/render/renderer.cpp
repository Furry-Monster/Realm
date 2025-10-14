#include "renderer.h"

#include "render/render_pipeline.h"
#include "utils.h"

#include <memory>
#include <string>

namespace RealmEngine
{
    void Renderer::initialize()
    {
        m_rhi.initialize();
        info("Renderer initalized.");
    }

    void Renderer::disposal()
    {
        m_render_scene.reset();
        m_render_pipeline.reset();
        m_rhi.disposal();

        info("Renderer disposed all resources.");
    }

    void Renderer::renderFrame()
    {

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
                type_str          = "Forward Rendering Pipeline";
                break;
            case PipelineType::Deffered:
                m_render_pipeline.reset();
                m_render_pipeline = std::make_unique<DefferedPipeline>();
                type_str          = "Deffered Rendering Pipeline";
                break;
            default:
                err("Unknown pipeline type.");
                break;
        }

        if (!type_str.empty())
            info("Pipeline switched to <" + type_str + ">.");
    }

} // namespace RealmEngine