#pragma once

#include "render/render_resource.h"
#include "render/render_scene.h"
#include "render/rhi.h"
namespace RealmEngine
{
    class RenderPass
    {
    public:
        RenderPass()                   = default;
        virtual ~RenderPass() noexcept = default;

        RenderPass(const RenderPass&)            = delete;
        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass(RenderPass&&)                 = delete;
        RenderPass& operator=(RenderPass&&)      = delete;

        virtual void initialize(RHI& rhi)                                            = 0;
        virtual void render(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources) = 0;
        virtual void dispose(RHI& rhi)                                               = 0;
    };

    class ShadowPass : public RenderPass
    {
        ShadowPass()                    = default;
        ~ShadowPass() noexcept override = default;

        void initialize(RHI& rhi) override;
        void render(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources) override;
        void dispose(RHI& rhi) override;

        TextureHandle getShadowMap() const { return m_shadow_map; }

    private:
        static constexpr uint32_t SHADOW_MAP_SIZE = 2048;

        FBOHandle     m_shadow_fbo {0};
        TextureHandle m_shadow_map {0};
        ProgramHandle m_shadow_shader {0};
    };

    class ForwardMainPass : public RenderPass
    {
    public:
        ForwardMainPass()                    = default;
        ~ForwardMainPass() noexcept override = default;

        void initialize(RHI& rhi) override;
        void render(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources) override;
        void dispose(RHI& rhi) override;

    private:
        void renderSkybox(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources);
        void renderOpaqueObjects(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources);
        void renderTransparentObjects(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources);

        ProgramHandle m_skybox_shader {0};
        VAOHandle     m_skybox_vao {0};
        BufferHandle  m_skybox_vbo {0};
    };

    class PostprocessPass : public RenderPass
    {
    public:
        PostprocessPass()                    = default;
        ~PostprocessPass() noexcept override = default;

        void initialize(RHI& rhi) override;
        void render(RHI& rhi, RenderScene& scene, RenderSwapBuffer& resources) override;
        void dispose(RHI& rhi) override;

    private:
        ProgramHandle m_post_process_shader {0};
        VAOHandle     m_screen_quad_vao {0};
        BufferHandle  m_screen_quad_vbo {0};
    };
} // namespace RealmEngine