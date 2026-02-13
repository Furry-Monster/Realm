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

        void loadModel(RHIDevice& device);
        void loadModel(RHIDevice& device, AssetManager& asset_mgr);
    };

} // namespace RealmEngine
