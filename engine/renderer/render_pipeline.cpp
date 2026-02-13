#include "renderer/render_pipeline.h"

#include "rhi/rhi_device.h"

namespace RealmEngine
{
    RenderPipeline::~RenderPipeline()
    {
        if (!m_passes.empty())
            dispose();
    }

    void RenderPipeline::addPass(std::unique_ptr<RenderPass> pass) { m_passes.push_back(std::move(pass)); }

    void RenderPipeline::initialize(RHIDevice& device)
    {
        for (auto& pass : m_passes)
            pass->init(device);
    }

    void RenderPipeline::execute(const RenderContext& ctx)
    {
        for (auto& pass : m_passes)
            pass->execute(ctx);
    }

    void RenderPipeline::dispose()
    {
        for (auto& pass : m_passes)
            pass->dispose();
        m_passes.clear();
    }

} // namespace RealmEngine
