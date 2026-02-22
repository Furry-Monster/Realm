#include "functional/render/render_pipeline.h"

#include "functional/render/rhi/rhi_device.h"

namespace RealmEngine
{
    RenderPipeline::~RenderPipeline() noexcept
    {
        if (!m_passes.empty())
            dispose();
    }

    void RenderPipeline::addPass(std::unique_ptr<RenderPass> pass) { m_passes.push_back(std::move(pass)); }

    void RenderPipeline::initialize(RHIDevice& device)
    {
        for (const auto& pass : m_passes)
            pass->init(device);
    }

    void RenderPipeline::execute(const RenderContext& ctx)
    {
        for (const auto& pass : m_passes)
            pass->execute(ctx);
    }

    void RenderPipeline::dispose()
    {
        for (const auto& pass : m_passes)
            pass->dispose();
        m_passes.clear();
    }

} // namespace RealmEngine
