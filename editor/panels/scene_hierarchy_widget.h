#pragma once

#include <memory>
#include "widget.h"

namespace RealmEngine
{
    class Scene;
    class SceneNode;
    class EditorContext;
    class SceneManager;

    class SceneHierarchyWidget : public Widget
    {
    public:
        SceneHierarchyWidget(std::shared_ptr<EditorContext> context, SceneManager& scene_mgr);
        ~SceneHierarchyWidget() override = default;

        SceneHierarchyWidget(const SceneHierarchyWidget&)            = delete;
        SceneHierarchyWidget& operator=(const SceneHierarchyWidget&) = delete;
        SceneHierarchyWidget(SceneHierarchyWidget&&)                 = default;
        SceneHierarchyWidget& operator=(SceneHierarchyWidget&&)      = default;

        void render() override;

    private:
        void renderNode(std::shared_ptr<SceneNode> node, Scene& scene);

        std::shared_ptr<EditorContext> m_context;
        SceneManager&                  m_scene_mgr;
    };

} // namespace RealmEngine
