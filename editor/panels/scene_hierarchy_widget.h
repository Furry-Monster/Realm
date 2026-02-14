#pragma once

#include <memory>
#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;
    class EditorContext;
    class Scene;
    class SceneNode;

    class SceneHierarchyWidget : public Widget
    {
    public:
        SceneHierarchyWidget(std::shared_ptr<EditorContext> context, EditorEngineBridge& bridge);
        ~SceneHierarchyWidget() override = default;

        SceneHierarchyWidget(const SceneHierarchyWidget&)            = delete;
        SceneHierarchyWidget& operator=(const SceneHierarchyWidget&) = delete;
        SceneHierarchyWidget(SceneHierarchyWidget&&)                 = delete;
        SceneHierarchyWidget& operator=(SceneHierarchyWidget&&)      = delete;

        void render() override;

    private:
        void renderNode(std::shared_ptr<SceneNode> node, Scene& scene);

        std::shared_ptr<EditorContext> m_context;
        EditorEngineBridge*            m_bridge;
    };

} // namespace RealmEngine
