#include "render_resource.h"

#include "utils.h"

namespace RealmEngine
{
    void RenderResource::initialize() { info("Render Resource Manager initialized."); }
    void RenderResource::disposal() { info("Render Resource Manager dispoed all resource."); }
} // namespace RealmEngine