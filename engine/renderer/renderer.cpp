#include "renderer/renderer.h"

#include <memory>

#include "core/log/log_macros.h"
#include "platform/window/window.h"
#include "renderer/fullscreen_quad.h"
#include "renderer/ibl/diffuse_irradiance_map.h"
#include "renderer/ibl/equirectangular_cubemap.h"
#include "renderer/ibl/specular_map.h"
#include "renderer/passes/bloom_pass.h"
#include "renderer/passes/geometry_pass.h"
#include "renderer/passes/hair_pass.h"
#include "renderer/passes/postprocess_pass.h"
#include "renderer/passes/shadow_pass.h"
#include "renderer/passes/skybox_pass.h"
#include "renderer/passes/ssao_blur_pass.h"
#include "renderer/passes/ssao_pass.h"
#include "renderer/passes/sss_pass.h"
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

        // Create RHI device (currently always OpenGL)
        m_device = RHIDevice::create();
        m_device->setDepthTest(true);
        m_device->enableSeamlessCubemap();
        m_device->enableMultisample(window.isMSAAEnabled());

        m_camera       = std::make_shared<RenderCamera>();
        m_render_scene = std::make_shared<RenderScene>();

        const RendererConfig& rc = config.getRendererConfig();
        m_camera->initialize();
        m_camera->setPerspective(rc.camera_fov,
                                 static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()),
                                 rc.camera_near_plane,
                                 rc.camera_far_plane);
        m_camera->setPosition(glm::vec3(rc.camera_initial_pos_x, rc.camera_initial_pos_y, rc.camera_initial_pos_z));
        m_camera->lookAt(glm::vec3(rc.camera_look_at_x, rc.camera_look_at_y, rc.camera_look_at_z));

        // IBL precomputation (still GL-specific internally)
        precomputeIBL(config);

        m_fullscreen_quad = std::make_unique<FullscreenQuad>(*m_device);

        // Build and initialize the render pipeline
        buildPipeline(config);

        m_device->setViewport(0, 0, window.getWidth(), window.getHeight());

        RE_LOG_INFO("Renderer initialized (RHI + pipeline).");
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

    void Renderer::buildPipeline(ConfigManager& config)
    {
        const RendererConfig& rc = config.getRendererConfig();
        std::string           sp = m_shader_path.generic_string();

        // --- Shadow pass ---
        auto shadow   = std::make_unique<ShadowPass>(sp, 2048);
        m_shadow_pass = shadow.get();
        m_pipeline.addPass(std::move(shadow));

        // --- Geometry pass (main PBR) ---
        auto geometry = std::make_unique<GeometryPass>(
            sp, rc.clear_color_r, rc.clear_color_g, rc.clear_color_b, rc.clear_color_a, rc.bloom_brightness_cutoff);
        m_geometry_pass = geometry.get();
        m_pipeline.addPass(std::move(geometry));

        // --- Hair pass (Kajiya-Kay + shell layers) ---
        auto hair   = std::make_unique<HairPass>(sp, rc.bloom_brightness_cutoff);
        m_hair_pass = hair.get();
        m_pipeline.addPass(std::move(hair));

        // --- SSAO pass ---
        auto ssao = std::make_unique<SSAOPass>(
            sp, rc.ssao_enabled, rc.ssao_radius, rc.ssao_bias, rc.ssao_kernel_size, rc.ssao_noise_size);
        m_ssao_pass = ssao.get();
        m_pipeline.addPass(std::move(ssao));

        // --- SSAO blur pass ---
        auto ssao_blur   = std::make_unique<SSAOBlurPass>(sp);
        m_ssao_blur_pass = ssao_blur.get();
        m_pipeline.addPass(std::move(ssao_blur));

        // --- SSS pass ---
        auto sss   = std::make_unique<SSSPass>(sp, rc.sss_enabled, rc.sss_radius, rc.sss_samples);
        m_sss_pass = sss.get();
        m_pipeline.addPass(std::move(sss));

        // --- Skybox pass ---
        auto skybox   = std::make_unique<SkyboxPass>(sp, rc.bloom_brightness_cutoff);
        m_skybox_pass = skybox.get();
        m_pipeline.addPass(std::move(skybox));

        // --- Bloom pass ---
        auto bloom   = std::make_unique<BloomPass>(sp,
                                                 rc.bloom_enabled,
                                                 rc.bloom_intensity,
                                                 rc.bloom_iterations,
                                                 static_cast<BloomDirection>(rc.bloom_direction));
        m_bloom_pass = bloom.get();
        m_pipeline.addPass(std::move(bloom));

        // --- Postprocess pass ---
        auto post = std::make_unique<PostProcessPass>(
            sp, rc.tonemapping_enabled, rc.gamma_correction_factor, rc.ssao_enabled, rc.ssao_power);
        m_postprocess_pass = post.get();
        m_pipeline.addPass(std::move(post));

        // Initialize all passes (compile shaders, create pass-owned resources)
        m_pipeline.initialize(*m_device);

        m_geometry_pass->setShadowPass(m_shadow_pass);
        m_geometry_pass->setIBLTextures(m_ibl_diffuse_tex, m_ibl_prefiltered_tex, m_ibl_brdf_tex);

        m_hair_pass->setGeometryPass(m_geometry_pass);
        m_hair_pass->setShadowPass(m_shadow_pass);
        m_hair_pass->setIBLTextures(m_ibl_diffuse_tex);

        // Create PBR framebuffer for geometry pass
        {
            FramebufferDesc desc;
            desc.width  = m_window->getWidth();
            desc.height = m_window->getHeight();

            FramebufferAttachment color0;
            color0.format     = TextureFormat::RGBA16F;
            color0.min_filter = TextureFilter::Linear;
            color0.mag_filter = TextureFilter::Linear;
            color0.wrap       = TextureWrap::ClampToEdge;

            FramebufferAttachment bloom_color;
            bloom_color.format     = TextureFormat::RGBA16F;
            bloom_color.min_filter = TextureFilter::LinearMipmapLinear;
            bloom_color.mag_filter = TextureFilter::Linear;
            bloom_color.wrap       = TextureWrap::ClampToEdge;
            bloom_color.gen_mips   = true;

            desc.color_attachments                = {color0, bloom_color};
            desc.has_depth                        = true;
            desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
            desc.depth_attachment.is_renderbuffer = false;

            m_geometry_pass->setFramebuffer(m_device->createFramebuffer(desc));
        }

        // Bloom framebuffers
        {
            auto make_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = m_window->getWidth();
                desc.height = m_window->getHeight();

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
            int w = m_window->getWidth(), h = m_window->getHeight();
            m_ssao_pass->setFramebuffer(make_ao_fb(w, h));
            m_ssao_blur_pass->setFramebuffer(make_ao_fb(w, h));
        }

        {
            auto make_sss_fb = [&]() {
                FramebufferDesc desc;
                desc.width  = m_window->getWidth();
                desc.height = m_window->getHeight();
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

        m_ssao_pass->setGeometryPass(m_geometry_pass);
        m_ssao_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_ssao_blur_pass->setSSAOPass(m_ssao_pass);
        m_ssao_blur_pass->setGeometryPass(m_geometry_pass);
        m_ssao_blur_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_sss_pass->setGeometryPass(m_geometry_pass);
        m_sss_pass->setFullscreenQuad(m_fullscreen_quad.get());

        m_skybox_pass->setSkybox(m_skybox.get());
        m_skybox_pass->setGeometryPass(m_geometry_pass);
        m_bloom_pass->setGeometryPass(m_geometry_pass);
        m_bloom_pass->setFullscreenQuad(m_fullscreen_quad.get());
        m_postprocess_pass->setGeometryPass(m_geometry_pass);
        m_postprocess_pass->setBloomPass(m_bloom_pass);
        m_postprocess_pass->setSSAOBlurPass(m_ssao_blur_pass);
        m_postprocess_pass->setFullscreenQuad(m_fullscreen_quad.get());
    }

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

        // Update camera aspect ratio
        if (m_camera)
            m_camera->setPerspective(m_camera->getFov(),
                                     static_cast<float>(width) / static_cast<float>(height),
                                     m_camera->getNearPlane(),
                                     m_camera->getFarPlane());

        // Recreate geometry pass framebuffer at new resolution
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

            FramebufferAttachment bloom_color;
            bloom_color.format     = TextureFormat::RGBA16F;
            bloom_color.min_filter = TextureFilter::LinearMipmapLinear;
            bloom_color.mag_filter = TextureFilter::Linear;
            bloom_color.wrap       = TextureWrap::ClampToEdge;
            bloom_color.gen_mips   = true;

            desc.color_attachments                = {color0, bloom_color};
            desc.has_depth                        = true;
            desc.depth_attachment.format          = TextureFormat::Depth24Stencil8;
            desc.depth_attachment.is_renderbuffer = false;

            m_geometry_pass->setFramebuffer(m_device->createFramebuffer(desc));
        }

        // Recreate bloom framebuffers at new resolution
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

    void Renderer::disposal()
    {
        m_pipeline.dispose();

        if (m_render_scene)
        {
            m_render_scene->m_render_objects.clear();
            m_render_scene->m_render_model_matrices.clear();
        }

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
