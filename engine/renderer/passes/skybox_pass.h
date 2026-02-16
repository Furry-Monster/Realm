#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIFramebuffer;
    class RHITexture;
    class Skybox;
    class SceneColorSource;

    class SkyboxPass final : public RenderPass
    {
    public:
        explicit SkyboxPass(const std::string& shader_path);
        ~SkyboxPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSkybox(Skybox* skybox) { m_skybox = skybox; }
        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }

    private:
        std::string                m_shader_path;
        std::unique_ptr<RHIShader> m_shader;
        Skybox*                    m_skybox {nullptr};
        SceneColorSource*          m_scene_color {nullptr};
    };

} // namespace RealmEngine
