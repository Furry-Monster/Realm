#pragma once

#include "gameplay/scene.h"
#include "render/framebuffer.h"
#include "render/render_camera.h"
#include "render/shader.h"
#include <memory>
#include <string>

namespace RealmEngine
{
    class Window;

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

        std::shared_ptr<Window> mWindow;

        // framebuffers
        std::unique_ptr<Framebuffer> mFramebuffer;

        // scene
        std::shared_ptr<Scene> mScene;

        // camera
        std::shared_ptr<RenderCamera> mCamera;

        // shaders
        std::unique_ptr<Shader> mPbrShader;

        // shader paths
        std::string mShaderRootPath;
    };
} // namespace RealmEngine
