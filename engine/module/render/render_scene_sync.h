#pragma once

#include <memory>

namespace RealmEngine
{
    class Scene;
    class RenderScene;

    void syncFromScene(const std::shared_ptr<Scene>& scene, RenderScene& render_scene);
} // namespace RealmEngine
