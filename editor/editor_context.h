#pragma once

#include <memory>
#include "scene/entity.h"
#include "scene/scene_node.h"

namespace RealmEngine
{
    class EditorContext
    {
    public:
        EditorContext()  = default;
        ~EditorContext() = default;

        EditorContext(const EditorContext&)            = delete;
        EditorContext& operator=(const EditorContext&) = delete;
        EditorContext(EditorContext&&)                 = default;
        EditorContext& operator=(EditorContext&&)      = default;

        void                    setSelectedEntity(std::shared_ptr<Entity> entity);
        std::shared_ptr<Entity> getSelectedEntity() const;
        bool                    hasSelectedEntity() const;
        void                    clearSelectedEntity();

        void                       setSelectedNode(std::shared_ptr<SceneNode> node);
        std::shared_ptr<SceneNode> getSelectedNode() const;
        bool                       hasSelectedNode() const;
        void                       clearSelectedNode();

    private:
        std::shared_ptr<Entity>    m_selected_entity {nullptr};
        std::shared_ptr<SceneNode> m_selected_node {nullptr};
    };

} // namespace RealmEngine
