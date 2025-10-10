#include "render_pipeline.h"

#include "utils.h"

namespace RealmEngine
{
    void ForwardPipeline::initialize() { info("<Forward Rendering Pipeline> initialized"); }

    void ForwardPipeline::disposal() { info("<Forward Rendering Pipeline> disposed."); }

    void DefferedPipeline::initialize() { info("<Deffered Rendering Pipeline> initialized"); }

    void DefferedPipeline::disposal() { info("<Deffered Rendering Pipeline> disposed."); }

} // namespace RealmEngine