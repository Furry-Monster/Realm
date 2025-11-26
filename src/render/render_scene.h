#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "gameplay/scene_node.h"
#include "render/render_object.h"

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

        std::vector<std::shared_ptr<RenderObject>> m_render_objects;
        std::vector<glm::vec3>                     m_light_positions;
        std::vector<glm::vec3>                     m_light_colors;

    private:
        void syncNode(std::shared_ptr<Scene> scene, std::shared_ptr<SceneNode> node);
    };
} // namespace RealmEngine
