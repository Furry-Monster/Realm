#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class RHITexture;
    class SceneColorSource;

    class HiZPass final : public RenderPass
    {
    public:
        explicit HiZPass(const std::string& shader_path);
        ~HiZPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setSceneColorSource(SceneColorSource* src) { m_scene_color = src; }

        RHITexture* getHiZTexture() const { return m_hiz_texture.get(); }

    private:
        void ensureTexture(RHIDevice& device, int width, int height);
        int  computeMipCount(int width, int height) const;

        std::string m_shader_path;

        std::unique_ptr<RHIShader>  m_compute_shader;
        std::unique_ptr<RHITexture> m_hiz_texture;
        int                         m_tex_width {0};
        int                         m_tex_height {0};
        int                         m_mip_count {0};
        SceneColorSource*           m_scene_color {nullptr};
    };

} // namespace RealmEngine
