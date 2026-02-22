#include "module/render/components/renderable.h"

#include "functional/render/render_object.h"
#include "functional/resource/asset_manager.h"
#include "functional/resource/model_loader.h"

namespace RealmEngine
{
    void loadRenderableModel(Renderable& renderable, RHIDevice& device)
    {
        if (renderable.model_path.empty())
            return;

        auto meshes = ModelLoader::load(renderable.model_path, renderable.flip_textures, device, nullptr);
        if (!meshes.empty())
            renderable.render_object = std::make_shared<RenderObject>(std::move(meshes));
    }

    void loadRenderableModel(Renderable& renderable, RHIDevice& device, AssetManager& asset_mgr)
    {
        if (!renderable.model_path.empty())
            renderable.render_object =
                asset_mgr.getOrLoadModel(renderable.model_path, renderable.flip_textures, device);
    }

} // namespace RealmEngine
