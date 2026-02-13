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
    class GeometryPass;

    class SkyboxPass final : public RenderPass
    {
    public:
        SkyboxPass(const std::string& shader_path, float bloom_brightness_cutoff);
        ~SkyboxPass() override;

        void initialize(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSkybox(Skybox* skybox) { m_skybox = skybox; }
        void setGeometryPass(GeometryPass* gp) { m_geometry_pass = gp; }

    private:
        std::string                m_shader_path;
        float                      m_bloom_brightness_cutoff;
        std::unique_ptr<RHIShader> m_shader;
        Skybox*                    m_skybox {nullptr};
        GeometryPass*              m_geometry_pass {nullptr};
    };

} // namespace RealmEngine
