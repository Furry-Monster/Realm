#include "render_resource.h"

#include "utils.h"

namespace RealmEngine
{
    void RenderResource::initialize() { info("Render Resource Manager initialized."); }
    void RenderResource::disposal() { info("Render Resource Manager dispoed all resource."); }

    void RenderResource::update(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) {}
} // namespace RealmEngine