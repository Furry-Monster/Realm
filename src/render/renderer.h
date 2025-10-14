#pragma once

#include "render/render_pipeline.h"
#include "render/render_resource.h"
#include "render/render_scene.h"
#include "render/render_swap_buffer.h"
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

        void setPipeline(PipelineType type);

    private:
        void processSwapData();

        std::shared_ptr<RHI>              m_rhi;
        std::shared_ptr<RenderResource>   m_render_res;
        std::shared_ptr<RenderSwapBuffer> m_render_swap_buf;
        std::shared_ptr<RenderPipeline>   m_render_pipeline;
        std::shared_ptr<RenderScene>      m_render_scene;
        std::shared_ptr<RenderCamera>     m_render_camera;
    };
} // namespace RealmEngine