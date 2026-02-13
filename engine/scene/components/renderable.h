#pragma once

#include <memory>
#include <string>

namespace RealmEngine
{
    class RenderObject;

    struct Renderable
    {
        std::shared_ptr<RenderObject> render_object;
        std::string                   model_path;
        bool                          flip_textures = false;

        // Load model from model_path (call after setting model_path)
        void loadModel();
    };

} // namespace RealmEngine
