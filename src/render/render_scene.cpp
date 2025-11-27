#include "render/render_scene.h"

#include "gameplay/components/lighting.h"
#include "gameplay/components/renderable.h"
#include "gameplay/components/transform.h"
#include "gameplay/scene/scene.h"
#include "gameplay/scene/scene_manager.h"
#include "gameplay/scene/scene_node.h"
#include "global_context.h"

namespace RealmEngine
{
    void RenderScene::syncFromScene(std::shared_ptr<Scene> scene)
    {
        if (!scene)
            return;

        m_render_objects.clear();
        m_light_positions.clear();
        m_light_colors.clear();

        auto root = scene->getRoot();
        if (root)
            syncNode(scene, root);
    }

    void RenderScene::syncFromCurrentScene()
    {
        auto scene = g_context.m_scene->getCurrentScene();
        syncFromScene(scene);
    }

    void RenderScene::syncNode(std::shared_ptr<Scene> scene, std::shared_ptr<SceneNode> node)
    {
        if (!node || !scene)
            return;

        if (node->hasEntity())
        {
            size_t entity_id = node->getEntityId();
            auto   entity    = scene->getEntity(entity_id);

            if (entity)
            {
                auto renderable = entity->getComponent<Renderable>();
                if (renderable && renderable->hasRenderObject())
                {
                    auto render_obj = renderable->getRenderObject();
                    if (render_obj)
                    {
                        auto transform = entity->getComponent<Transform>();
                        if (transform)
                        {
                            render_obj->setPosition(transform->getPosition());
                            render_obj->setOrientation(transform->getRotation());
                            render_obj->setScale(transform->getScale());
                        }

                        m_render_objects.push_back(render_obj);
                    }
                }

                auto lighting = entity->getComponent<Lighting>();
                if (lighting)
                {
                    glm::vec3 position = lighting->getPosition();
                    glm::vec3 color    = lighting->getColor();

                    auto transform = entity->getComponent<Transform>();
                    if (transform)
                        position = transform->getPosition();

                    m_light_positions.push_back(position);
                    m_light_colors.push_back(color);
                }
            }
        }

        node->forEachChild([this, scene](std::shared_ptr<SceneNode> child) { syncNode(scene, child); });
    }

} // namespace RealmEngine
