#pragma once

#include <cstdint>
#include <entt/entity/entity.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "renderer/light.h"
#include "renderer/render_object.h"

namespace RealmEngine
{
    class Scene;
    class SceneNode;

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
        std::vector<Light>                         m_lights;

    private:
        void fullSync(Scene& scene);
        void syncNode(Scene& scene, std::shared_ptr<SceneNode> node);
        void updateTransformsOnly(Scene& scene);

        std::shared_ptr<Scene>    m_cached_scene;
        uint64_t                  m_cached_generation {0};
        std::vector<entt::entity> m_render_entities;
        std::vector<entt::entity> m_light_entities;
    };
} // namespace RealmEngine
