#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"
#include "rhi/rhi_types.h"

namespace RealmEngine
{
    class RHIShader;
    class RHIBuffer;
    class RHITexture;
    class SceneColorSource;
    class ShadowPass;

    class HairPass final : public RenderPass
    {
    public:
        explicit HairPass(const std::string& shader_path);
        ~HairPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }
        void setShadowPass(ShadowPass* sp) { m_shadow_pass = sp; }
        void setIBLTextures(RHITexture* diffuse_irradiance);

    private:
        std::string m_shader_path;

        std::unique_ptr<RHIShader> m_shader;
        std::unique_ptr<RHIBuffer> m_light_ubo;

        SceneColorSource* m_scene_color {nullptr};
        ShadowPass*       m_shadow_pass {nullptr};
        RHITexture*       m_ibl_diffuse {nullptr};
    };

} // namespace RealmEngine
