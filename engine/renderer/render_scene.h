#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "scene/scene_node.h"
#include "renderer/light.h"
#include "renderer/render_object.h"

namespace RealmEngine
{
    class Scene;

    class RenderScene
    {
    public:
        RenderScene()           = default;
        ~RenderScene() noexcept = default;

        RenderScene(const RenderScene&)                = delete;
        RenderScene& operator=(const RenderScene&)     = delete;
        RenderScene(RenderScene&&) noexcept            = default;
        RenderScene& operator=(RenderScene&&) noexcept = default;

        void syncFromScene(std::shared_ptr<Scene> scene);
        void syncFromCurrentScene();

        std::vector<std::shared_ptr<RenderObject>> m_render_objects;
        std::vector<Light>                         m_lights;

    private:
        void syncNode(std::shared_ptr<Scene> scene, std::shared_ptr<SceneNode> node);
    };
} // namespace RealmEngine
