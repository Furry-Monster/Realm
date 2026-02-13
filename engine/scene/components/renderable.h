#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class RenderObject;
    class RHIDevice;

    struct Renderable
    {
        std::shared_ptr<RenderObject> render_object;
        std::string                   model_path;
        bool                          flip_textures = false;

        // Load model from model_path (requires an initialized RHI device)
        void loadModel(RHIDevice& device);
    };

} // namespace RealmEngine
