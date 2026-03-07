#include "functional/render/render_scene_sync.h"

#include "functional/ecs/components/transform.h"
#include "functional/ecs/components/world_transform.h"
#include "functional/render/components/lighting/area.h"
#include "functional/render/components/lighting/directional.h"
#include "functional/render/components/lighting/point.h"
#include "functional/render/components/lighting/spot.h"
#include "functional/render/components/renderable.h"
#include "functional/render/light.h"
#include "functional/render/render_scene.h"
#include "functional/scene/scene.h"
#include "functional/scene/scene_node.h"

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>

namespace RealmEngine
{
    namespace
    {
        struct SyncState
        {
            std::shared_ptr<Scene>        cached_scene;
            uint64_t                      cached_generation {0};
            std::vector<entt::entity>     render_entities;
            std::vector<entt::entity>     light_entities;
            std::vector<entt::connection> connections;
        };
        static SyncState s_state;

        glm::mat4 getModelMatrix(Scene& scene, entt::entity entity)
        {
            if (const auto* wt = scene.tryGet<WorldTransform>(entity))
                return wt->matrix;
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->getModelMatrix();
            return glm::mat4(1.0f);
        }

        glm::vec3 getWorldPosition(Scene& scene, entt::entity entity)
        {
            if (auto* wt = scene.tryGet<WorldTransform>(entity))
                return glm::vec3(wt->matrix[3]);
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->position;
            return glm::vec3(0.0f);
        }

        glm::vec3 getWorldForward(Scene& scene, entt::entity entity)
        {
            if (auto* wt = scene.tryGet<WorldTransform>(entity))
                return -glm::vec3(wt->matrix[0][2], wt->matrix[1][2], wt->matrix[2][2]);
            if (const auto* t = scene.tryGet<Transform>(entity))
                return t->getForward();
            return glm::vec3(0.0f, 0.0f, -1.0f);
        }

        void sceneMarkDirty(Scene* s, entt::registry&, entt::entity)
        {
            if (s)
                s->markDirty();
        }

        void connectSceneDirty(Scene* scene)
        {
            if (!scene)
                return;
            auto& reg = scene->getRegistry();
            s_state.connections.push_back(reg.on_construct<Renderable>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_construct<PointLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_construct<SpotLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_construct<AreaLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_construct<DirectionalLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_destroy<Renderable>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_destroy<PointLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_destroy<SpotLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_destroy<AreaLight>().connect<&sceneMarkDirty>(scene));
            s_state.connections.push_back(reg.on_destroy<DirectionalLight>().connect<&sceneMarkDirty>(scene));
        }

        void fullSync(Scene& scene, RenderScene& render_scene);
        void syncNode(Scene& scene, std::shared_ptr<SceneNode> node, RenderScene& render_scene);
        void updateTransformsOnly(Scene& scene, RenderScene& render_scene);
    } // namespace

    void clearSyncState()
    {
        s_state.connections.clear();
        s_state.cached_scene.reset();
        s_state.cached_generation = 0;
        s_state.render_entities.clear();
        s_state.light_entities.clear();
    }

    void syncFromScene(const std::shared_ptr<Scene>& scene, RenderScene& render_scene)
    {
        if (!scene)
        {
            render_scene.getRenderObjects().clear();
            render_scene.getRenderModelMatrices().clear();
            render_scene.getLights().clear();
            clearSyncState();
            return;
        }

        if (scene != s_state.cached_scene)
        {
            s_state.connections.clear();
            connectSceneDirty(scene.get());
            s_state.cached_scene = scene;
        }

        const bool structure_changed = (scene->getGeneration() != s_state.cached_generation);

        if (structure_changed)
        {
            fullSync(*scene, render_scene);
            s_state.cached_generation = scene->getGeneration();
        }
        else
        {
            updateTransformsOnly(*scene, render_scene);
        }
    }

    namespace
    {
        void fullSync(Scene& scene, RenderScene& render_scene)
        {
            render_scene.getRenderObjects().clear();
            render_scene.getRenderModelMatrices().clear();
            render_scene.getLights().clear();
            s_state.render_entities.clear();
            s_state.light_entities.clear();

            const auto root = scene.getRoot();
            if (root)
                syncNode(scene, root, render_scene);
        }

        void syncNode(Scene& scene, std::shared_ptr<SceneNode> node, RenderScene& render_scene)
        {
            if (!node)
                return;

            if (node->hasEntity())
            {
                entt::entity entity = node->getEntity();
                if (!scene.valid(entity))
                {
                    node->forEachChild([&scene, &render_scene](const std::shared_ptr<SceneNode>& child) {
                        syncNode(scene, child, render_scene);
                    });
                    return;
                }

                if (auto* r = scene.tryGet<Renderable>(entity))
                {
                    if (r->render_object)
                    {
                        render_scene.getRenderObjects().push_back(r->render_object);
                        render_scene.getRenderModelMatrices().push_back(getModelMatrix(scene, entity));
                        s_state.render_entities.push_back(entity);
                    }
                }

                if (auto* pl = scene.tryGet<PointLight>(entity))
                {
                    if (pl->enabled)
                    {
                        Light light {};
                        light.type             = LightType::Point;
                        light.position         = getWorldPosition(scene, entity);
                        light.direction        = glm::vec3(0.0f);
                        light.color            = pl->color;
                        light.intensity        = pl->intensity;
                        light.constant         = pl->constant;
                        light.linear           = pl->linear;
                        light.quadratic        = pl->quadratic;
                        light.range            = pl->range;
                        light.inner_cone_angle = 0.0f;
                        light.outer_cone_angle = 0.0f;
                        light.width            = 0.0f;
                        light.height           = 0.0f;
                        render_scene.getLights().push_back(light);
                        s_state.light_entities.push_back(entity);
                    }
                }

                if (auto* sl = scene.tryGet<SpotLight>(entity))
                {
                    if (sl->enabled)
                    {
                        Light light {};
                        light.type             = LightType::Spot;
                        light.position         = getWorldPosition(scene, entity);
                        light.direction        = getWorldForward(scene, entity);
                        light.color            = sl->color;
                        light.intensity        = sl->intensity;
                        light.constant         = sl->constant;
                        light.linear           = sl->linear;
                        light.quadratic        = sl->quadratic;
                        light.range            = sl->range;
                        light.inner_cone_angle = sl->inner_cone_angle;
                        light.outer_cone_angle = sl->outer_cone_angle;
                        light.width            = 0.0f;
                        light.height           = 0.0f;
                        render_scene.getLights().push_back(light);
                        s_state.light_entities.push_back(entity);
                    }
                }

                if (auto* al = scene.tryGet<AreaLight>(entity))
                {
                    if (al->enabled)
                    {
                        Light light {};
                        light.type             = LightType::Area;
                        light.position         = getWorldPosition(scene, entity);
                        light.direction        = getWorldForward(scene, entity);
                        light.color            = al->color;
                        light.intensity        = al->intensity;
                        light.constant         = 0.0f;
                        light.linear           = 0.0f;
                        light.quadratic        = 0.0f;
                        light.range            = std::sqrt(al->width * al->width + al->height * al->height) * 2.0f;
                        light.inner_cone_angle = 0.0f;
                        light.outer_cone_angle = 0.0f;
                        light.width            = al->width;
                        light.height           = al->height;
                        render_scene.getLights().push_back(light);
                        s_state.light_entities.push_back(entity);
                    }
                }

                if (auto* dl = scene.tryGet<DirectionalLight>(entity))
                {
                    if (dl->enabled)
                    {
                        Light light {};
                        light.type             = LightType::Directional;
                        light.position         = glm::vec3(0.0f);
                        light.direction        = getWorldForward(scene, entity);
                        light.color            = dl->color;
                        light.intensity        = dl->intensity;
                        light.constant         = 0.0f;
                        light.linear           = 0.0f;
                        light.quadratic        = 0.0f;
                        light.range            = 0.0f;
                        light.inner_cone_angle = 0.0f;
                        light.outer_cone_angle = 0.0f;
                        light.width            = 0.0f;
                        light.height           = 0.0f;
                        render_scene.getLights().push_back(light);
                        s_state.light_entities.push_back(entity);
                    }
                }
            }

            node->forEachChild([&scene, &render_scene](const std::shared_ptr<SceneNode>& child) {
                syncNode(scene, child, render_scene);
            });
        }

        void updateTransformsOnly(Scene& scene, RenderScene& render_scene)
        {
            auto& matrices = render_scene.getRenderModelMatrices();
            auto& lights   = render_scene.getLights();

            for (size_t i = 0; i < s_state.render_entities.size(); ++i)
            {
                const entt::entity entity = s_state.render_entities[i];
                if (!scene.valid(entity) || i >= render_scene.getRenderObjects().size() || i >= matrices.size())
                    continue;
                matrices[i] = getModelMatrix(scene, entity);
            }

            for (size_t i = 0; i < s_state.light_entities.size(); ++i)
            {
                const entt::entity entity = s_state.light_entities[i];
                if (!scene.valid(entity) || i >= lights.size())
                    continue;

                Light& light = lights[i];

                if (const auto* pl = scene.tryGet<PointLight>(entity))
                {
                    light.position  = getWorldPosition(scene, entity);
                    light.color     = pl->color;
                    light.intensity = pl->enabled ? pl->intensity : 0.0f;
                    light.constant  = pl->constant;
                    light.linear    = pl->linear;
                    light.quadratic = pl->quadratic;
                    light.range     = pl->range;
                }
                else if (const auto* sl = scene.tryGet<SpotLight>(entity))
                {
                    light.position         = getWorldPosition(scene, entity);
                    light.direction        = getWorldForward(scene, entity);
                    light.color            = sl->color;
                    light.intensity        = sl->enabled ? sl->intensity : 0.0f;
                    light.constant         = sl->constant;
                    light.linear           = sl->linear;
                    light.quadratic        = sl->quadratic;
                    light.range            = sl->range;
                    light.inner_cone_angle = sl->inner_cone_angle;
                    light.outer_cone_angle = sl->outer_cone_angle;
                }
                else if (const auto* al = scene.tryGet<AreaLight>(entity))
                {
                    light.position  = getWorldPosition(scene, entity);
                    light.direction = getWorldForward(scene, entity);
                    light.color     = al->color;
                    light.intensity = al->enabled ? al->intensity : 0.0f;
                    light.width     = al->width;
                    light.height    = al->height;
                    light.range     = std::sqrt(al->width * al->width + al->height * al->height) * 2.0f;
                }
                else if (const auto* dl = scene.tryGet<DirectionalLight>(entity))
                {
                    light.direction = getWorldForward(scene, entity);
                    light.color     = dl->color;
                    light.intensity = dl->enabled ? dl->intensity : 0.0f;
                }
            }
        }
    } // namespace
} // namespace RealmEngine
