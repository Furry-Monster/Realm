#pragma once

#include <memory>
#include <string>

#include "renderer/render_pass.h"

namespace RealmEngine
{
    class RHIShader;
    class FullscreenQuad;
    class GeometryPass;
    class BloomPass;

    class PostProcessPass final : public RenderPass
    {
    public:
        PostProcessPass(const std::string& shader_path, bool tonemapping, float gamma);
        ~PostProcessPass() override;

        void init(RHIDevice& device) override;
        void execute(const RenderContext& ctx) override;
        void dispose() override;

        void setGeometryPass(GeometryPass* gp) { m_geometry_pass = gp; }
        void setBloomPass(BloomPass* bp) { m_bloom_pass = bp; }
        void setFullscreenQuad(FullscreenQuad* quad) { m_quad = quad; }

    private:
        std::string                m_shader_path;
        bool                       m_tonemapping;
        float                      m_gamma;
        std::unique_ptr<RHIShader> m_shader;
        GeometryPass*              m_geometry_pass {nullptr};
        BloomPass*                 m_bloom_pass {nullptr};
        FullscreenQuad*            m_quad {nullptr};
    };

} // namespace RealmEngine
