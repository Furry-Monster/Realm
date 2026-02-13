#include "scene/components/renderable.h"

#include "renderer/render_object.h"
#include "resource/asset_manager.h"
#include "resource/model_loader.h"

namespace RealmEngine
{
    void Renderable::loadModel(RHIDevice& device)
    {
        if (model_path.empty())
            return;

        auto meshes = ModelLoader::load(model_path, flip_textures, device, nullptr);
        if (!meshes.empty())
            render_object = std::make_shared<RenderObject>(std::move(meshes));
    }

    void Renderable::loadModel(RHIDevice& device, AssetManager& asset_mgr)
    {
        if (!model_path.empty())
            render_object = asset_mgr.getOrLoadModel(model_path, flip_textures, device);
    }

} // namespace RealmEngine
