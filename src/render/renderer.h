#pragma once

#include "render/render_pipeline.h"
#include "render/render_scene.h"
#include "render/resource_manager.h"
#include <memory>

namespace RealmEngine
{
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

    private:
        RenderScene               m_render_scene;
        GraphicResourceManager    m_grahic_res_mgr;
        std::unique_ptr<RenderPipeline> m_render_pipeline;
    };
} // namespace RealmEngine