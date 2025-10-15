#pragma once

#include "render/render_camera.h"
#include "render/render_scene.h"
#include "render/rhi.h"
#include "resource/datatype/model/material.h"
#include <memory>

namespace RealmEngine
{
    class RenderPipeline
    {
    public:
        virtual ~RenderPipeline() noexcept = default;

        virtual void initialize() = 0;
        virtual void render()     = 0;
        virtual void disposal()   = 0;
    };

    class ForwardPipeline : public RenderPipeline
    {
    public:
        ForwardPipeline()           = default;
        ~ForwardPipeline() noexcept = default;

        ForwardPipeline(const ForwardPipeline& that)            = delete;
        ForwardPipeline(ForwardPipeline&& that)                 = delete;
        ForwardPipeline& operator=(const ForwardPipeline& that) = delete;
        ForwardPipeline& operator=(ForwardPipeline&& that)      = delete;

        void initialize() override;
        void disposal() override;

        void render() override;

        // Set render context
        void setRenderContext(std::shared_ptr<RHI>          rhi,
                              std::shared_ptr<RenderScene>  scene,
                              std::shared_ptr<RenderCamera> camera);

    protected:
        void renderShadowMaps();
        void renderForward();
        void renderPostprocess();

        // Shader loading
        void loadShaders();
        void setupShaderUniforms();

        // Shadow mapping
        void setupShadowMapping();

        // Rendering helpers
        void renderObject(const RenderObject& obj);
        void setupMaterial(const Material& material);
        void setupLighting();
        void setupCameraUniforms();

    private:
        // Render context
        std::shared_ptr<RHI>          m_rhi;
        std::shared_ptr<RenderScene>  m_scene;
        std::shared_ptr<RenderCamera> m_camera;

        // Shader programs
        ProgramHandle m_pbr_program;
        ProgramHandle m_shadow_program;
        ProgramHandle m_skybox_program;

        // Shadow mapping
        FBOHandle      m_shadow_fbo;
        TextureHandle  m_shadow_map;
        const uint32_t SHADOW_WIDTH  = 1024;
        const uint32_t SHADOW_HEIGHT = 1024;
    };

    class DefferedPipeline : public RenderPipeline
    {
    public:
        DefferedPipeline()           = default;
        ~DefferedPipeline() noexcept = default;

        DefferedPipeline(const DefferedPipeline& that)            = delete;
        DefferedPipeline(DefferedPipeline&& that)                 = delete;
        DefferedPipeline& operator=(const DefferedPipeline& that) = delete;
        DefferedPipeline& operator=(DefferedPipeline&& that)      = delete;

        void initialize() override;
        void disposal() override;

        void render() override;

    protected:
    };
} // namespace RealmEngine