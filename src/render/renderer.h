#pragma once

#include "gameplay/scene.h"
#include "render/bloom_framebuffer.h"
#include "render/framebuffer.h"
#include "render/fullscreen_quad.h"
#include "render/ibl/diffuse_irradiance_map.h"
#include "render/ibl/equirectangular_cubemap.h"
#include "render/ibl/specular_map.h"
#include "render/render_camera.h"
#include "render/shader.h"
#include "render/skybox.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Window;

    enum class BloomDirection
    {
        BOTH      = 0,
        HORIZONTAL = 1,
        VERTICAL   = 2
    };

    /**
     * Manages rendering.
     */
    class Renderer
    {
    public:
        Renderer()  = default;
        ~Renderer() = default;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;

        void initialize(std::shared_ptr<Window> window);
        void shutdown();
        void render(std::shared_ptr<Scene> scene);

        std::shared_ptr<RenderCamera> getCamera() const { return mCamera; }

    private:
        void setupShaders();
        void setupFramebuffers();
        void setupIBL();
        void renderBloom();

        std::shared_ptr<Window> mWindow;

        // framebuffers
        std::unique_ptr<Framebuffer>        mFramebuffer;
        std::unique_ptr<BloomFramebuffer>   mBloomFramebuffers[2];
        unsigned int                        mBloomFramebufferResult;

        // pre-computed IBL stuff
        std::unique_ptr<EquirectangularCubemap> mIblEquirectangularCubemap;
        std::unique_ptr<DiffuseIrradianceMap>   mIblDiffuseIrradianceMap;
        std::unique_ptr<SpecularMap>            mIblSpecularMap;

        // skybox
        std::unique_ptr<Skybox> mSkybox;

        // scene
        std::shared_ptr<Scene> mScene;

        // camera
        std::shared_ptr<RenderCamera> mCamera;

        // shaders
        std::unique_ptr<Shader> mPbrShader;
        std::unique_ptr<Shader> mBloomShader;
        std::unique_ptr<Shader> mPostShader;
        std::unique_ptr<Shader> mSkyboxShader;

        // post-processing
        std::unique_ptr<FullscreenQuad> mFullscreenQuad;
        bool                            mBloomEnabled          = true;
        float                           mBloomIntensity        = 1.0f;
        int                             mBloomIterations       = 10;
        BloomDirection                  mBloomDirection        = BloomDirection::BOTH;
        bool                            mTonemappingEnabled     = false;
        float                           mGammaCorrectionFactor = 2.2f;
        float                           mBloomBrightnessCutoff  = 1.0f;

        // paths
        std::string mShaderRootPath;
        std::string mEngineRootPath;
        std::string mHdriPath;

        // IBL texture units
        static const int TEXTURE_UNIT_DIFFUSE_IRRADIANCE_MAP = 10;
        static const int TEXTURE_UNIT_PREFILTERED_ENV_MAP    = 11;
        static const int TEXTURE_UNIT_BRDF_CONVOLUTION_MAP   = 12;
    };
} // namespace RealmEngine
