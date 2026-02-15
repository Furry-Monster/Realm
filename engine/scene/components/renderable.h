#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class AssetManager;
    class RenderObject;
    class RHIDevice;

    struct Renderable
    {
        std::shared_ptr<RenderObject> render_object;
        std::string                   model_path;
        bool                          flip_textures = false;
    };

    // Resource loading helpers (kept separate from the component to maintain ECS data purity)
    void loadRenderableModel(Renderable& renderable, RHIDevice& device);
    void loadRenderableModel(Renderable& renderable, RHIDevice& device, AssetManager& asset_mgr);

} // namespace RealmEngine
