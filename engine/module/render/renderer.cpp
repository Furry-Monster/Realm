#include "module/render/renderer.h"

#include <memory>

#include "core/base/macros.h"
#include "functional/render/fullscreen_quad.h"
#include "functional/render/ibl/diffuse_irradiance_map.h"
#include "functional/render/ibl/equirectangular_cubemap.h"
#include "functional/render/ibl/specular_map.h"
#include "functional/render/passes/bloom_pass.h"
#include "functional/render/passes/clustered_light_cull_pass.h"
#include "functional/render/passes/csm_shadow_pass.h"
#include "functional/render/passes/deferred_lighting_pass.h"
#include "functional/render/passes/gbuffer_pass.h"
#include "functional/render/passes/gtao_blur_pass.h"
#include "functional/render/passes/gtao_pass.h"
#include "functional/render/passes/hiz_pass.h"
#include "functional/render/passes/opaque_pass.h"
#include "functional/render/passes/point_shadow_pass.h"
#include "functional/render/passes/postprocess_pass.h"
#include "functional/render/passes/skybox_pass.h"
#include "functional/render/passes/spot_shadow_pass.h"
#include "functional/render/passes/ssr_pass.h"
#include "functional/render/passes/transparent_pass.h"
#include "functional/render/rhi/rhi_device.h"
#include "functional/render/rhi/rhi_framebuffer.h"
#include "functional/render/rhi/rhi_texture.h"
#include "functional/render/rhi/rhi_types.h"
#include "functional/render/scene_color_source.h"
#include "functional/render/skybox.h"
#include "functional/resource/config_manager.h"
#include "module/render/light_probe_baker.h"
#include "module/render/render_scene_sync.h"
#include "platform/window/window.h"

namespace RealmEngine
{
    Renderer::Renderer()           = default;
    Renderer::~Renderer() noexcept = default;

    void Renderer::initialize(const ConfigManager& config, Window& window)
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

        m_probe_baker = std::make_unique<LightProbeBaker>(*m_device);
        m_probe_baker->initShader(m_shader_path.string());

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

        constexpr uint8_t white[] = {255, 255, 255, 255};
        TextureDesc       td;
        td.type         = TextureType::Texture2D;
        td.format       = TextureFormat::RGBA8;
        td.width        = 1;
        td.height       = 1;
        td.data         = white;
        td.min_filter   = TextureFilter::Nearest;
        td.mag_filter   = TextureFilter::Nearest;
        m_default_white = m_device->createTexture(td);

        if (m_pipeline_mode == PipelineMode::Deferred)
            RE_LOG_INFO("Renderer initialized (RHI + Deferred pipeline).");
        else
            RE_LOG_INFO("Renderer initialized (RHI + Forward pipeline).");
    }

    void Renderer::precomputeIBL(const ConfigManager& config)
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

    void Renderer::buildForwardPipeline(const ConfigManager& config)
    {
        const RendererConfig& rc = config.getRendererConfig();
        std::string           sp = m_shader_path.generic_string();

        auto shadow   = std::make_unique<CSMShadowPass>(sp, 2048);
        m_shadow_pass = shadow.get();
        m_pipeline.addPass(std::move(shadow));

        auto point_shadow   = std::make_unique<PointShadowPass>(sp, 1024);
        m_point_shadow_pass = point_shadow.get();
        m_pipeline.addPass(std::move(point_shadow));

        auto spot_shadow   = std::make_unique<SpotShadowPass>(sp, 1024);
        m_spot_shadow_pass = spot_shadow.get();
        m_pipeline.addPass(std::move(spot_shadow));

        auto opaque =
            std::make_unique<OpaquePass>(sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a);
        m_opaque_pass        = opaque.get();
        m_scene_color_source = opaque.get();
        m_pipeline.addPass(std::move(opaque));

        auto transparent   = std::make_unique<TransparentPass>(sp);
        m_transparent_pass = transparent.get();
        m_pipeline.addPass(std::move(transparent));

        auto gtao =
            std::make_unique<GTAOPass>(sp, rc.ao_enabled, rc.ao_radius, rc.gtao_num_directions, rc.gtao_num_steps);
        m_gtao_pass = gtao.get();
        m_pipeline.addPass(std::move(gtao));

        auto gtao_blur   = std::make_unique<GTAOBlurPass>(sp);
        m_gtao_blur_pass = gtao_blur.get();
        m_pipeline.addPass(std::move(gtao_blur));

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
            sp, rc.tonemapping_enabled, rc.gamma_correction_factor, rc.ao_enabled, rc.ao_power, rc.ao_intensity);
        m_postprocess_pass = post.get();
        m_pipeline.addPass(std::move(post));

        m_pipeline.initialize(*m_device);

        // Cross-pass wiring
        m_opaque_pass->setShadowPass(m_shadow_pass);
        m_opaque_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        m_transparent_pass->setSceneColorSource(m_opaque_pass);
        m_transparent_pass->setShadowPass(m_shadow_pass);
        m_transparent_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        // Framebuffers
        createSharedFramebuffers(m_window->getWidth(), m_window->getHeight(), rc);

        m_gtao_pass->setSceneColorSource(m_opaque_pass);
        m_gtao_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_gtao_blur_pass->setGTAOPass(m_gtao_pass);
        m_gtao_blur_pass->setSceneColorSource(m_opaque_pass);
        m_gtao_blur_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_skybox_pass->setSkybox(m_skybox.get());
        m_skybox_pass->setSceneColorSource(m_opaque_pass);
        m_bloom_pass->setSceneColorSource(m_opaque_pass);
        m_bloom_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_postprocess_pass->setSceneColorSource(m_opaque_pass);
        m_postprocess_pass->setBloomPass(m_bloom_pass);
        m_postprocess_pass->setGTAOBlurPass(m_gtao_blur_pass);
        m_postprocess_pass->setFullscreenQuad(m_fullscreen_quad.get());
    }

    // ---- Deferred pipeline --------------------------------------------------

    void Renderer::buildDeferredPipeline(const ConfigManager& config)
    {
        const RendererConfig& rc = config.getRendererConfig();
        std::string           sp = m_shader_path.generic_string();

        auto shadow   = std::make_unique<CSMShadowPass>(sp, 2048);
        m_shadow_pass = shadow.get();
        m_pipeline.addPass(std::move(shadow));

        auto point_shadow   = std::make_unique<PointShadowPass>(sp, 1024);
        m_point_shadow_pass = point_shadow.get();
        m_pipeline.addPass(std::move(point_shadow));

        auto spot_shadow   = std::make_unique<SpotShadowPass>(sp, 1024);
        m_spot_shadow_pass = spot_shadow.get();
        m_pipeline.addPass(std::move(spot_shadow));

        auto cluster_cull   = std::make_unique<ClusteredLightCullPass>(sp);
        m_cluster_cull_pass = cluster_cull.get();
        m_pipeline.addPass(std::move(cluster_cull));

        auto gbuffer =
            std::make_unique<GBufferPass>(sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a);
        m_gbuffer_pass = gbuffer.get();
        m_pipeline.addPass(std::move(gbuffer));

        auto hiz   = std::make_unique<HiZPass>(sp);
        m_hiz_pass = hiz.get();
        m_pipeline.addPass(std::move(hiz));

        auto deferred_lighting   = std::make_unique<DeferredLightingPass>(sp);
        m_deferred_lighting_pass = deferred_lighting.get();
        m_scene_color_source     = deferred_lighting.get();
        m_pipeline.addPass(std::move(deferred_lighting));

        auto ssr   = std::make_unique<SSRPass>(sp, true, 64, 100.0f);
        m_ssr_pass = ssr.get();
        m_pipeline.addPass(std::move(ssr));

        auto gtao =
            std::make_unique<GTAOPass>(sp, rc.ao_enabled, rc.ao_radius, rc.gtao_num_directions, rc.gtao_num_steps);
        m_gtao_pass = gtao.get();
        m_pipeline.addPass(std::move(gtao));

        auto gtao_blur   = std::make_unique<GTAOBlurPass>(sp);
        m_gtao_blur_pass = gtao_blur.get();
        m_pipeline.addPass(std::move(gtao_blur));

        // Forward passes for non-deferred materials (custom, transparent)
        auto opaque =
            std::make_unique<OpaquePass>(sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a);
        m_opaque_pass = opaque.get();
        m_pipeline.addPass(std::move(opaque));

        auto transparent   = std::make_unique<TransparentPass>(sp);
        m_transparent_pass = transparent.get();
        m_pipeline.addPass(std::move(transparent));

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
            sp, rc.tonemapping_enabled, rc.gamma_correction_factor, rc.ao_enabled, rc.ao_power, rc.ao_intensity);
        m_postprocess_pass = post.get();
        m_pipeline.addPass(std::move(post));

        m_pipeline.initialize(*m_device);

        // Cross-pass wiring
        m_deferred_lighting_pass->setGBufferPass(m_gbuffer_pass);
        m_deferred_lighting_pass->setShadowPass(m_shadow_pass);
        m_deferred_lighting_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);
        m_deferred_lighting_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_hiz_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_ssr_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_ssr_pass->setHiZPass(m_hiz_pass);
        m_ssr_pass->setGBufferPass(m_gbuffer_pass);
        m_ssr_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_gtao_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_gtao_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_gtao_blur_pass->setGTAOPass(m_gtao_pass);
        m_gtao_blur_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_gtao_blur_pass->setFullscreenQuad(m_fullscreen_quad.get());

        // In deferred mode, OpaquePass renders non-deferred-eligible opaques
        // into the deferred lighting output framebuffer
        m_opaque_pass->setDeferredMode(true, m_deferred_lighting_pass);
        m_opaque_pass->setShadowPass(m_shadow_pass);
        m_opaque_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        m_transparent_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_transparent_pass->setShadowPass(m_shadow_pass);
        m_transparent_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        // Framebuffers
        createSharedFramebuffers(m_window->getWidth(), m_window->getHeight(), rc);

        m_skybox_pass->setSkybox(m_skybox.get());
        m_skybox_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_bloom_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_bloom_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_postprocess_pass->setSceneColorSource(m_deferred_lighting_pass);
        m_postprocess_pass->setBloomPass(m_bloom_pass);
        m_postprocess_pass->setGTAOBlurPass(m_gtao_blur_pass);
        m_postprocess_pass->setFullscreenQuad(m_fullscreen_quad.get());
    }

    // ---- Shared framebuffer creation ----------------------------------------

    void Renderer::createSharedFramebuffers(int width, int height, [[maybe_unused]] const RendererConfig& rc)
    {
        if (m_pipeline_mode == PipelineMode::Forward)
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

            m_opaque_pass->setFramebuffer(m_device->createFramebuffer(desc));
        }
        else
        {
            // G-Buffer: 4 color RTs (albedo+modelID, normal+metallic, emissive+roughness, ao) + Depth
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;

                FramebufferAttachment rt;
                rt.format     = TextureFormat::RGBA16F;
                rt.min_filter = TextureFilter::Nearest;
                rt.mag_filter = TextureFilter::Nearest;
                rt.wrap       = TextureWrap::ClampToEdge;

                desc.color_attachments                = {rt, rt, rt, rt};
                desc.has_depth                        = true;
                desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
                desc.depth_attachment.is_renderbuffer = false;

                m_gbuffer_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }

            // Deferred lighting output
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

            // OpaquePass in deferred mode uses DeferredLightingPass's framebuffer
            // via setDeferredMode(), no dedicated framebuffer needed.

            // SSR output framebuffer
            if (m_ssr_pass)
            {
                FramebufferDesc desc;
                desc.width  = width;
                desc.height = height;
                FramebufferAttachment color;
                color.format           = TextureFormat::RGBA16F;
                color.min_filter       = TextureFilter::Linear;
                color.mag_filter       = TextureFilter::Linear;
                color.wrap             = TextureWrap::ClampToEdge;
                desc.color_attachments = {color};
                m_ssr_pass->setFramebuffer(m_device->createFramebuffer(desc));
            }
        }

        // GTAO framebuffers
        if (m_gtao_pass && m_gtao_blur_pass)
        {
            auto make_ao_fb = [&](const int w, const int h) {
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
            m_gtao_pass->setFramebuffer(make_ao_fb(width, height));
            m_gtao_blur_pass->setFramebuffer(make_ao_fb(width, height));
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
    }

    void Renderer::recreateSharedFramebuffers(const int width, const int height)
    {
        const RendererConfig dummy;
        createSharedFramebuffers(width, height, dummy);
    }

    // ---- Render / Resize / Disposal -----------------------------------------

    void Renderer::render()
    {
        if (!m_render_scene)
        {
            RE_LOG_ERROR("Render scene not set.");
            return;
        }

        if (m_default_white)
        {
            for (uint32_t u = 0; u < 8; ++u)
                m_device->bindTexture(u, *m_default_white);
        }

        RenderContext ctx;
        ctx.device               = m_device.get();
        ctx.scene                = m_render_scene.get();
        ctx.ecs_scene            = m_current_ecs_scene;
        ctx.camera               = m_camera.get();
        ctx.viewport_width       = m_window->getWidth();
        ctx.viewport_height      = m_window->getHeight();
        ctx.display_mode         = m_display_mode;
        ctx.viewport_framebuffer = m_render_to_viewport_texture ? m_viewport_framebuffer.get() : nullptr;

        m_pipeline.execute(ctx);
    }

    void Renderer::onResize(const int width, const int height)
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

    void Renderer::setRenderToViewportTexture(const bool enable)
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

    RHITexture* Renderer::getGBufferTexture(const GBufferSlot slot) const
    {
        if (m_pipeline_mode != PipelineMode::Deferred || !m_gbuffer_pass)
            return nullptr;
        auto* fb = m_gbuffer_pass->getFramebuffer();
        if (!fb)
            return nullptr;
        switch (slot)
        {
            case GBufferSlot::AlbedoModelID:
                return fb->getColorAttachment(0);
            case GBufferSlot::NormalMetallic:
                return fb->getColorAttachment(1);
            case GBufferSlot::EmissiveRoughness:
                return fb->getColorAttachment(2);
            case GBufferSlot::Depth:
                return fb->getDepthAttachment();
            default:
                return nullptr;
        }
    }

    void Renderer::reloadCustomShaders()
    {
        if (m_opaque_pass)
            m_opaque_pass->reloadShaders();
        if (m_transparent_pass)
            m_transparent_pass->reloadShaders();
    }

    void Renderer::disposal()
    {
        clearSyncState();

        m_pipeline.dispose();

        if (m_render_scene)
        {
            m_render_scene->getRenderObjects().clear();
            m_render_scene->getRenderModelMatrices().clear();
        }

        m_scene_color_source = nullptr;

        m_ibl_diffuse_tex     = nullptr;
        m_ibl_prefiltered_tex = nullptr;
        m_ibl_brdf_tex        = nullptr;

        m_ibl_equirect.reset();
        m_ibl_diffuse.reset();
        m_ibl_specular.reset();
        m_probe_baker.reset();
        m_skybox.reset();
        m_fullscreen_quad.reset();
        m_default_white.reset();

        m_camera->disposal();
        m_camera.reset();
        m_render_scene.reset();
        m_window = nullptr;

        m_viewport_framebuffer.reset();
        m_device.reset();

        RE_LOG_INFO("Renderer shutdown.");
    }

} // namespace RealmEngine
