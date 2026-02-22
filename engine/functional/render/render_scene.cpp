#include "functional/render/render_scene.h"

namespace RealmEngine
{
    int RenderScene::getDrawCallCount() const
    {
        int count = 0;
        for (const auto& ro : m_render_objects)
        {
            if (ro)
                count += static_cast<int>(ro->getMeshCount());
        }
        return count;
    }

    int RenderScene::getTriangleCount() const
    {
        int count = 0;
        for (const auto& ro : m_render_objects)
        {
            if (ro)
            {
                for (size_t i = 0; i < ro->getMeshCount(); ++i)
                    count += ro->getTriangleCount(i);
            }
        }
        return count;
    }

    std::optional<std::reference_wrapper<const Light>> RenderScene::findDirectionalLight() const
    {
        for (const Light& light : m_lights)
        {
            if (light.type == LightType::Directional)
                return std::cref(light);
        }
        return std::nullopt;
    }
} // namespace RealmEngine
