#pragma once

#include <memory>
#include "editor/widget.h"

namespace RealmEngine
{
    class SceneNode;
    class EditorContext;

    class SceneHierarchyWidget : public Widget
    {
    public:
        explicit SceneHierarchyWidget(std::shared_ptr<EditorContext> context);
        ~SceneHierarchyWidget() override = default;

        SceneHierarchyWidget(const SceneHierarchyWidget&)            = delete;
        SceneHierarchyWidget& operator=(const SceneHierarchyWidget&) = delete;
        SceneHierarchyWidget(SceneHierarchyWidget&&)                 = default;
        SceneHierarchyWidget& operator=(SceneHierarchyWidget&&)      = default;

        void render() override;

    private:
        void renderNode(std::shared_ptr<SceneNode> node);

        std::shared_ptr<EditorContext> m_context;
    };

} // namespace RealmEngine
