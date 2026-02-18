#pragma once

#include <functional>
#include <memory>
#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;
    class EditorContext;
    class Scene;
    class SceneNode;

    struct HierarchyCallbacks
    {
        std::function<void()> on_delete;
        std::function<void()> on_duplicate;
    };

    class SceneHierarchyWidget : public Widget
    {
    public:
        SceneHierarchyWidget(const std::shared_ptr<EditorContext>& context,
                             EditorEngineBridge&                   bridge,
                             HierarchyCallbacks                    callbacks = {});
        ~SceneHierarchyWidget() override = default;

        SceneHierarchyWidget(const SceneHierarchyWidget&)            = delete;
        SceneHierarchyWidget& operator=(const SceneHierarchyWidget&) = delete;
        SceneHierarchyWidget(SceneHierarchyWidget&&)                 = delete;
        SceneHierarchyWidget& operator=(SceneHierarchyWidget&&)      = delete;

        void render() override;

    private:
        void renderNode(const std::shared_ptr<SceneNode>& node, Scene& scene);

        std::shared_ptr<EditorContext> m_context;
        EditorEngineBridge*            m_bridge;
        HierarchyCallbacks             m_callbacks;

        std::weak_ptr<SceneNode> m_renaming_node;
        char                     m_rename_buffer[128];
    };

} // namespace RealmEngine
