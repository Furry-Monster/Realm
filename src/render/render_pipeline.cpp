#include "render_pipeline.h"

#include "utils.h"

namespace RealmEngine
{
    /// -----------------------
    ///     ForwardPipeline
    /// -----------------------
    void ForwardPipeline::initialize() { info("<Forward Rendering Pipeline> initialized"); }

    void ForwardPipeline::disposal() { info("<Forward Rendering Pipeline> disposed."); }

    void ForwardPipeline::render()
    {
        renderShadowMaps();
        renderForward();
        renderPostprocess();
    }

    void ForwardPipeline::renderShadowMaps() {}

    void ForwardPipeline::renderForward() {}

    void ForwardPipeline::renderPostprocess() {}

    /// -----------------------
    ///     DefferedPipeline
    /// -----------------------

    void DefferedPipeline::initialize() { info("<Deffered Rendering Pipeline> initialized"); }

    void DefferedPipeline::disposal() { info("<Deffered Rendering Pipeline> disposed."); }

    void DefferedPipeline::render() {}

} // namespace RealmEngine