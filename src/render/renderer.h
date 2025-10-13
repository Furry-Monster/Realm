#pragma once

#include "render/render_pipeline.h"
#include "render/render_scene.h"
#include "render/rhi.h"
#include <cstdint>
#include <memory>

namespace RealmEngine
{
    enum class PipelineType : uint8_t
    {
        Forward,
        Deffered,
    };

    class Renderer
    {
    public:
        Renderer()           = default;
        ~Renderer() noexcept = default;

        Renderer(const Renderer& that)            = delete;
        Renderer(Renderer&& that)                 = delete;
        Renderer& operator=(const Renderer& that) = delete;
        Renderer& operator=(Renderer&& that)      = delete;

        void initialize();
        void disposal();

        void renderFrame();

        void setRenderScene();
        void setPipeline(PipelineType type);

    private:
        RHI                             m_rhi;
        std::unique_ptr<RenderScene>    m_render_scene;
        std::unique_ptr<RenderPipeline> m_render_pipeline;
    };
} // namespace RealmEngine