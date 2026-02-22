#pragma once

#include <memory>

namespace RealmEngine
{
    class Scene;
    class RenderScene;

    void syncFromScene(const std::shared_ptr<Scene>& scene, RenderScene& render_scene);

    // NOTE:
    // Clear static sync state so Scene is not held past GL context lifetime
    // (call before renderer/window disposal).
    void clearSyncState();
} // namespace RealmEngine
