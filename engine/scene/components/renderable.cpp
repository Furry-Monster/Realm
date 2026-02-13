#include "scene/components/renderable.h"

#include "renderer/render_object.h"

namespace RealmEngine
{
    void Renderable::loadModel()
    {
        if (!model_path.empty())
            render_object = std::make_shared<RenderObject>(model_path, flip_textures);
    }

} // namespace RealmEngine
