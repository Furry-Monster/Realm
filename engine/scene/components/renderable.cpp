#include "scene/components/renderable.h"

#include "renderer/render_object.h"
#include "resource/asset_manager.h"

namespace RealmEngine
{
    void Renderable::loadModel(RHIDevice& device)
    {
        if (!model_path.empty())
            render_object = std::make_shared<RenderObject>(model_path, flip_textures, device);
    }

    void Renderable::loadModel(RHIDevice& device, AssetManager& asset_mgr)
    {
        if (!model_path.empty())
            render_object = asset_mgr.getOrLoadModel(model_path, flip_textures, device);
    }

} // namespace RealmEngine
