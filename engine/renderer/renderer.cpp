#include "renderer/renderer.h"

#include <memory>

#include "core/log/log_macros.h"
#include "platform/window/window.h"
#include "renderer/fullscreen_quad.h"
#include "renderer/ibl/diffuse_irradiance_map.h"
#include "renderer/ibl/equirectangular_cubemap.h"
#include "renderer/ibl/specular_map.h"
#include "renderer/passes/bloom_pass.h"
#include "renderer/passes/deferred_lighting_pass.h"
#include "renderer/passes/gbuffer_pass.h"
#include "renderer/passes/geometry_pass.h"
#include "renderer/passes/hair_pass.h"
#include "renderer/passes/postprocess_pass.h"
#include "renderer/passes/shadow_pass.h"
#include "renderer/passes/skybox_pass.h"
#include "renderer/passes/ssao_blur_pass.h"
#include "renderer/passes/ssao_pass.h"
#include "renderer/passes/sss_pass.h"
#include "renderer/scene_color_source.h"
#include "renderer/skybox.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "rhi/rhi_framebuffer.h"
#include "rhi/rhi_texture.h"

namespace RealmEngine
{
    Renderer::Renderer()           = default;
    Renderer::~Renderer() noexcept = default;

    void Renderer::initialize(ConfigManager& config, Window& window)
    {
        m_window      = &window;
        m_shader_path = config.getShaderFolder();

        m_device = RHIDevice::create();
        m_device->setDepthTest(true);
        m_device->enableSeamlessCubemap();
        m_device->enableMultisample(window.isMSAAEnabled());

        m_camera       = std::make_shared<RenderCamera>();
        m_render_scene = std::make_shared<RenderScene>();

        const RendererConfig& rc = config.getRendererConfig();
        m_pipeline_mode          = rc.pipeline_mode;

        m_camera->initialize();
        m_camera->setPerspective(rc.camera_fov,
                                 static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()),
                                 rc.camera_near_plane,
                                 rc.camera_far_plane);
        m_camera->setPosition(glm::vec3(rc.camera_initial_pos_x, rc.camera_initial_pos_y, rc.camera_initial_pos_z));
        m_camera->lookAt(glm::vec3(rc.camera_look_at_x, rc.camera_look_at_y, rc.camera_look_at_z));

        precomputeIBL(config);

        m_fullscreen_quad = std::make_unique<FullscreenQuad>(*m_device);

        switch (m_pipeline_mode)
        {
            case PipelineMode::Forward:
                buildForwardPipeline(config);
                break;
            case PipelineMode::Deferred:
                buildDeferredPipeline(config);
                break;
        }

        m_device->setViewport(0, 0, window.getWidth(), window.getHeight());

        if (m_pipeline_mode == PipelineMode::Deferred)
            RE_LOG_INFO("Renderer initialized (RHI + Deferred pipeline).");
        else
            RE_LOG_INFO("Renderer initialized (RHI + Forward pipeline).");
    }

    void Renderer::precomputeIBL(ConfigManager& config)
    {
        std::string root_path = config.getRootFolder().generic_string();
        std::string hdri_path = (config.getAssetFolder() / config.getRendererConfig().hdri_path).generic_string();

        m_ibl_equirect = std::make_unique<EquirectangularCubemap>(*m_device, root_path, hdri_path);
        m_ibl_equirect->compute(*m_device);

        m_ibl_diffuse =
            std::make_unique<DiffuseIrradianceMap>(*m_device, root_path, m_ibl_equirect->getCubemapTexture());
        m_ibl_diffuse->compute(*m_device);

        m_ibl_specular = std::make_unique<SpecularMap>(*m_device, root_path, m_ibl_equirect->getCubemapTexture());
        m_ibl_specular->computePrefilteredEnvMap(*m_device);
        m_ibl_specular->computeBrdfConvolutionMap(*m_device);

        RHITexture* env_cubemap = m_ibl_equirect->getCubemapTexture();
        m_skybox                = std::make_unique<Skybox>(*m_device, env_cubemap);

        m_ibl_diffuse_tex     = m_ibl_diffuse->getCubemapTexture();
        m_ibl_prefiltered_tex = m_ibl_specular->getPrefilteredEnvMapTexture();
        m_ibl_brdf_tex        = m_ibl_specular->getBrdfConvolutionTexture();
    }

    // ---- Forward pipeline ---------------------------------------------------

    void Renderer::buildForwardPipeline(ConfigManager& config)
    {
        const RendererConfig& rc = config.getRendererConfig();
        std::string           sp = m_shader_path.generic_string();

        auto shadow   = std::make_unique<ShadowPass>(sp, 2048);
        m_shadow_pass = shadow.get();
        m_pipeline.addPass(std::move(shadow));

        auto geometry =
            std::make_unique<GeometryPass>(sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a);
        m_geometry_pass      = geometry.get();
        m_scene_color_source = geometry.get();
        m_pipeline.addPass(std::move(geometry));

        auto hair   = std::make_unique<HairPass>(sp);
        m_hair_pass = hair.get();
        m_pipeline.addPass(std::move(hair));

        auto ssao = std::make_unique<SSAOPass>(
            sp, rc.ssao_enabled, rc.ssao_radius, rc.ssao_bias, rc.ssao_kernel_size, rc.ssao_noise_size);
        m_ssao_pass = ssao.get();
        m_pipeline.addPass(std::move(ssao));

        auto ssao_blur   = std::make_unique<SSAOBlurPass>(sp);
        m_ssao_blur_pass = ssao_blur.get();
        m_pipeline.addPass(std::move(ssao_blur));

        auto sss   = std::make_unique<SSSPass>(sp, rc.sss_enabled, rc.sss_radius, rc.sss_samples);
        m_sss_pass = sss.get();
        m_pipeline.addPass(std::move(sss));

        auto skybox   = std::make_unique<SkyboxPass>(sp);
        m_skybox_pass = skybox.get();
        m_pipeline.addPass(std::move(skybox));

        auto bloom   = std::make_unique<BloomPass>(sp,
                                                 rc.bloom_enabled,
                                                 rc.bloom_intensity,
                                                 rc.bloom_brightness_cutoff,
                                                 rc.bloom_iterations,
                                                 static_cast<BloomDirection>(rc.bloom_direction));
        m_bloom_pass = bloom.get();
        m_pipeline.addPass(std::move(bloom));

        auto post = std::make_unique<PostProcessPass>(
            sp, rc.tonemapping_enabled, rc.gamma_correction_factor, rc.ssao_enabled, rc.ssao_power);
        m_postprocess_pass = post.get();
        m_pipeline.addPass(std::move(post));

        m_pipeline.initialize(*m_device);

        // Cross-pass wiring
        m_geometry_pass->setShadowPass(m_shadow_pass);
        m_geometry_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        m_hair_pass->setSceneColorSource(m_geometry_pass);
        m_hair_pass->setShadowPass(m_shadow_pass);
        m_hair_pass->setIBLTextures(m_ibl_diffuse_tex);

        // Framebuffers
        createSharedFramebuffers(m_window->getWidth(), m_window->getHeight(), rc);

        m_ssao_pass->setSceneColorSource(m_geometry_pass);
        m_ssao_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_ssao_blur_pass->setSSAOPass(m_ssao_pass);
        m_ssao_blur_pass->setSceneColorSource(m_geometry_pass);
        m_ssao_blur_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_sss_pass->setSceneColorSource(m_geometry_pass);
        m_sss_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_skybox_pass->setSkybox(m_skybox.get());
        m_skybox_pass->setSceneColorSource(m_geometry_pass);
        m_bloom_pass->setSceneColorSource(m_geometry_pass);
        m_bloom_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_postprocess_pass->setSceneColorSource(m_geometry_pass);
        m_postprocess_pass->setBloomPass(m_bloom_pass);
        m_postprocess_pass->setSSAOBlurPass(m_ssao_blur_pass);
        m_postprocess_pass->setFullscreenQuad(m_fullscreen_quad.get());
    }

    // ---- Deferred pipeline --------------------------------------------------

    void Renderer::buildDeferredPipeline(ConfigManager& config)
    {
        const RendererConfig& rc = config.getRendererConfig();
        std::string           sp = m_shader_path.generic_string();

        // Shadow
        auto shadow   = std::make_unique<ShadowPass>(sp, 2048);
        m_shadow_pass = shadow.get();
        m_pipeline.addPass(std::move(shadow));

        // G-Buffer
        auto gbuffer =
            std::make_unique<GBufferPass>(sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a);
        m_gbuffer_pass = gbuffer.get();
        m_pipeline.addPass(std::move(gbuffer));

        // Deferred lighting
        auto deferred_lighting   = std::make_unique<DeferredLightingPass>(sp);
        m_deferred_lighting_pass = deferred_lighting.get();
        m_scene_color_source     = deferred_lighting.get();
        m_pipeline.addPass(std::move(deferred_lighting));

        // Hair (forward, renders into lighting framebuffer)
        auto hair   = std::make_unique<HairPass>(sp);
        m_hair_pass = hair.get();
        m_pipeline.addPass(std::move(hair));

        // SSAO
        auto ssao = std::make_unique<SSAOPass>(
            sp, rc.ssao_enabled, rc.ssao_radius, rc.ssao_bias, rc.ssao_kernel_size, rc.ssao_noise_size);
        m_ssao_pass = ssao.get();
        m_pipeline.addPass(std::move(ssao));

        auto ssao_blur   = std::make_unique<SSAOBlurPass>(sp);
        m_ssao_blur_pass = ssao_blur.get();
        m_pipeline.addPass(std::move(ssao_blur));

        // SSS is forward-only, skip in deferred

        // Skybox
        auto skybox   = std::make_unique<SkyboxPass>(sp);
        m_skybox_pass = skybox.get();
        m_pipeline.addPass(std::move(skybox));

        // Bloom
        auto bloom   = std::make_unique<BloomPass>(sp,
                                                 rc.bloom_enabled,
                                                 rc.bloom_intensity,
                                                 rc.bloom_brightness_cutoff,
                                                 rc.bloom_iterations,
                                                 static_cast<BloomDirection>(rc.bloom_direction));
        m_bloom_pass = bloom.get();
        m_pipeline.addPass(std::move(bloom));

        // Post-process
        auto post = std::make_unique<PostProcessPass>(
            sp, rc.tonemapping_enabled, rc.gamma_correction_factor, rc.ssao_enabled, rc.ssao_power);
        m_postprocess_pass = post.get();
        m_pipeline.addPass(std::move(post));

        m_pipeline.initialize(*m_device);

        // Cross-pass wiring
        m_deferred_lighting_pass->setGBufferPass(m_gbuffer_pass);
        m_deferred_lighting_pass->setShadowPass(m_shadow_pass);
        m_deferred_lighting_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);
        m_deferred_lighting_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_hair_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_hair_pass->setShadowPass(m_shadow_pass);
        m_hair_pass->setIBLTextures(m_ibl_diffuse_tex);

        // Framebuffers
        createSharedFramebuffers(m_window->getWidth(), m_window->getHeight(), rc);

        m_ssao_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_ssao_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_ssao_blur_pass->setSSAOPass(m_ssao_pass);
        m_ssao_blur_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_ssao_blur_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_skybox_pass->setSkybox(m_skybox.get());
        m_skybox_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_bloom_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_bloom_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_postprocess_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_postprocess_pass->setBloomPass(m_bloom_pass);
        m_postprocess_pass->setSSAOBlurPass(m_ssao_blur_pass);
        m_postprocess_pass->setFullscreenQuad(m_fullscreen_quad.get());
    }

    // ---- Shared framebuffer creation ----------------------------------------

    void Renderer::createSharedFramebuffers(int width, int height, const RendererConfig& /*rc*/)
    {
        if (m_pipeline_mode == PipelineMode::Forward)
        {
            // Geometry pass framebuffer: RGBA16F + Depth24Stencil8
            FramebufferDesc desc;
            desc.width  = width;
            desc.height = height;

            FramebufferAttachment color0;
            color0.format     = TextureFormat::RGBA16F;
            color0.min_filter = TextureFilter::Linear;
            color0.mag_filter = TextureFilter::Linear;
            color0.wrap       = TextureWrap::ClampToEdge;

            desc.color_attachments                = {color0};
            desc.has_depth                        = true;
            desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
            desc.depth_attachment.is_renderbuffer = false;

            m_geometry_pass->setFramebuffer(m_device->createFramebuffer(desc));
        }
        else
        {
            // G-Buffer: 3 color RTs + Depth
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment rt;
                rt.format     = TextureFormat::RGBA16F;
                rt.min_filter = TextureFilter::Nearest;
                rt.mag_filter = TextureFilter::Nearest;
                rt.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {rt, rt, rt};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_gbuffer_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }

            // Deferred lighting output: RGBA16F + Depth24Stencil8
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment color0;
                color0.format     = TextureFormat::RGBA16F;
                color0.min_filter = TextureFilter::Linear;
                color0.mag_filter = TextureFilter::Linear;
                color0.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {color0};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_deferred_lighting_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }
        }

        // Bloom framebuffers
        if (m_bloom_pass)
        {
            auto make_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment color;
                color.format           = TextureFormat::RGBA16F;
                color.min_filter       = TextureFilter::LinearMipmapLinear;
                color.mag_filter       = TextureFilter::Linear;
                color.wrap             = TextureWrap::ClampToEdge;
                color.gen_mips         = true;
                desc.color_attachments = {color};

                return m_device->createFramebuffer(desc);
            };
            m_bloom_pass->setFramebuffers(make_fb(), make_fb());
        }

        // SSAO framebuffers
        if (m_ssao_pass && m_ssao_blur_pass)
        {
            auto make_ao_fb = [&](int w, int h) {
                FramebufferDesc desc;
                desc.width  = w;
                desc.height = h;
                FramebufferAttachment ao;
                ao.format              = TextureFormat::R16F;
                ao.min_filter          = TextureFilter::Linear;
                ao.mag_filter          = TextureFilter::Linear;
                ao.wrap                = TextureWrap::ClampToEdge;
                desc.color_attachments = {ao};
                return m_device->createFramebuffer(desc);
            };
            m_ssao_pass->setFramebuffer(make_ao_fb(width, height));
            m_ssao_blur_pass->setFramebuffer(make_ao_fb(width, height));
        }

        // SSS framebuffers (forward only)
        if (m_sss_pass)
        {
            auto make_sss_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;
                FramebufferAttachment color;
                color.format           = TextureFormat::RGBA16F;
                color.min_filter       = TextureFilter::Linear;
                color.mag_filter       = TextureFilter::Linear;
                color.wrap             = TextureWrap::ClampToEdge;
                desc.color_attachments = {color};
                return m_device->createFramebuffer(desc);
            };
            m_sss_pass->setFramebuffers(make_sss_fb(), make_sss_fb());
        }
    }

    void Renderer::recreateSharedFramebuffers(int width, int height)
    {
        if (m_pipeline_mode == PipelineMode::Forward)
        {
            if (m_geometry_pass)
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment color0;
                color0.format     = TextureFormat::RGBA16F;
                color0.min_filter = TextureFilter::Linear;
                color0.mag_filter = TextureFilter::Linear;
                color0.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {color0};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_geometry_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }
        }
        else
        {
            if (m_gbuffer_pass)
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment rt;
                rt.format     = TextureFormat::RGBA16F;
                rt.min_filter = TextureFilter::Nearest;
                rt.mag_filter = TextureFilter::Nearest;
                rt.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {rt, rt, rt};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_gbuffer_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }

            if (m_deferred_lighting_pass)
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment color0;
                color0.format     = TextureFormat::RGBA16F;
                color0.min_filter = TextureFilter::Linear;
                color0.mag_filter = TextureFilter::Linear;
                color0.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {color0};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_deferred_lighting_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }
        }

        if (m_bloom_pass)
        {
            auto make_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment color;
                color.format           = TextureFormat::RGBA16F;
                color.min_filter       = TextureFilter::LinearMipmapLinear;
                color.mag_filter       = TextureFilter::Linear;
                color.wrap             = TextureWrap::ClampToEdge;
                color.gen_mips         = true;
                desc.color_attachments = {color};

                return m_device->createFramebuffer(desc);
            };
            m_bloom_pass->setFramebuffers(make_fb(), make_fb());
        }

        if (m_ssao_pass && m_ssao_blur_pass)
        {
            auto make_ao_fb = [&](int w, int h) {
                FramebufferDesc desc;
                desc.width  = w;
                desc.height = h;
                FramebufferAttachment ao;
                ao.format              = TextureFormat::R16F;
                ao.min_filter          = TextureFilter::Linear;
                ao.mag_filter          = TextureFilter::Linear;
                ao.wrap                = TextureWrap::ClampToEdge;
                desc.color_attachments = {ao};
                return m_device->createFramebuffer(desc);
            };
            m_ssao_pass->setFramebuffer(make_ao_fb(width, height));
            m_ssao_blur_pass->setFramebuffer(make_ao_fb(width, height));
        }

        if (m_sss_pass)
        {
            auto make_sss_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;
                FramebufferAttachment color;
                color.format           = TextureFormat::RGBA16F;
                color.min_filter       = TextureFilter::Linear;
                color.mag_filter       = TextureFilter::Linear;
                color.wrap             = TextureWrap::ClampToEdge;
                desc.color_attachments = {color};
                return m_device->createFramebuffer(desc);
            };
            m_sss_pass->setFramebuffers(make_sss_fb(), make_sss_fb());
        }
    }

    // ---- Render / Resize / Disposal -----------------------------------------

    void Renderer::render()
    {
        if (!m_render_scene)
        {
            RE_LOG_ERROR("Render scene not set.");
            return;
        }

        RenderContext ctx;
        ctx.device               = m_device.get();
        ctx.scene                = m_render_scene.get();
        ctx.camera               = m_camera.get();
        ctx.viewport_width       = m_window->getWidth();
        ctx.viewport_height      = m_window->getHeight();
        ctx.display_mode         = m_display_mode;
        ctx.viewport_framebuffer = m_render_to_viewport_texture ? m_viewport_framebuffer.get() : nullptr;

        m_pipeline.execute(ctx);
    }

    void Renderer::onResize(int width, int height)
    {
        if (width <= 0 || height <= 0)
            return;

        m_device->setViewport(0, 0, width, height);

        if (m_camera)
            m_camera->setPerspective(m_camera->getFov(),
                                     static_cast<float>(width) / static_cast<float>(height),
                                     m_camera->getNearPlane(),
                                     m_camera->getFarPlane());

        recreateSharedFramebuffers(width, height);

        if (m_viewport_framebuffer)
        {
            FramebufferDesc desc;
            desc.width  = width;
            desc.height = height;
            FramebufferAttachment color;
            color.format           = TextureFormat::RGBA8;
            color.min_filter       = TextureFilter::Linear;
            color.mag_filter       = TextureFilter::Linear;
            color.wrap             = TextureWrap::ClampToEdge;
            desc.color_attachments = {color};
            m_viewport_framebuffer = m_device->createFramebuffer(desc);
        }
    }

    void Renderer::setRenderToViewportTexture(bool enable)
    {
        m_render_to_viewport_texture = enable;
        if (enable && !m_viewport_framebuffer && m_window)
        {
            FramebufferDesc desc;
            desc.width  = m_window->getWidth();
            desc.height = m_window->getHeight();

            FramebufferAttachment color;
            color.format           = TextureFormat::RGBA8;
            color.min_filter       = TextureFilter::Linear;
            color.mag_filter       = TextureFilter::Linear;
            color.wrap             = TextureWrap::ClampToEdge;
            desc.color_attachments = {color};

            m_viewport_framebuffer = m_device->createFramebuffer(desc);
        }
        if (!enable)
            m_viewport_framebuffer.reset();
    }

    RHITexture* Renderer::getViewportTexture() const
    {
        return m_viewport_framebuffer ? m_viewport_framebuffer->getColorAttachment(0) : nullptr;
    }

    RHITexture* Renderer::getGBufferAlbedoAO() const
    {
        if (m_pipeline_mode != PipelineMode::Deferred || !m_gbuffer_pass)
            return nullptr;
        auto* fb = m_gbuffer_pass->getFramebuffer();
        return fb ? fb->getColorAttachment(0) : nullptr;
    }

    RHITexture* Renderer::getGBufferNormalMetallic() const
    {
        if (m_pipeline_mode != PipelineMode::Deferred || !m_gbuffer_pass)
            return nullptr;
        auto* fb = m_gbuffer_pass->getFramebuffer();
        return fb ? fb->getColorAttachment(1) : nullptr;
    }

    RHITexture* Renderer::getGBufferEmissiveRoughness() const
    {
        if (m_pipeline_mode != PipelineMode::Deferred || !m_gbuffer_pass)
            return nullptr;
        auto* fb = m_gbuffer_pass->getFramebuffer();
        return fb ? fb->getColorAttachment(2) : nullptr;
    }

    RHITexture* Renderer::getGBufferDepth() const
    {
        if (m_pipeline_mode != PipelineMode::Deferred || !m_gbuffer_pass)
            return nullptr;
        auto* fb = m_gbuffer_pass->getFramebuffer();
        return fb ? fb->getDepthAttachment() : nullptr;
    }

    void Renderer::disposal()
    {
        m_pipeline.dispose();

        if (m_render_scene)
        {
            m_render_scene->m_render_objects.clear();
            m_render_scene->m_render_model_matrices.clear();
        }

        m_scene_color_source = nullptr;

        m_ibl_diffuse_tex     = nullptr;
        m_ibl_prefiltered_tex = nullptr;
        m_ibl_brdf_tex        = nullptr;

        m_ibl_equirect.reset();
        m_ibl_diffuse.reset();
        m_ibl_specular.reset();
        m_skybox.reset();
        m_fullscreen_quad.reset();

        m_camera->disposal();
        m_camera.reset();
        m_render_scene.reset();
        m_window = nullptr;

        m_viewport_framebuffer.reset();
        m_device.reset();

        RE_LOG_INFO("Renderer shutdown.");
    }

} // namespace RealmEngine
