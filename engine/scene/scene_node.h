#pragma once

#include <cstddef>
#include <entt/entity/entity.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace RealmEngine
{
    class SceneNode : public std::enable_shared_from_this<SceneNode>
    {
    public:
        SceneNode();
        explicit SceneNode(const std::string& name);
        virtual ~SceneNode() noexcept = default;

        SceneNode(const SceneNode&)                = delete;
        SceneNode& operator=(const SceneNode&)     = delete;
        SceneNode(SceneNode&&) noexcept            = default;
        SceneNode& operator=(SceneNode&&) noexcept = default;

        void               setName(const std::string& name);
        const std::string& getName() const;

        void         setEntity(entt::entity entity);
        entt::entity getEntity() const;
        bool         hasEntity() const;

        void                       addChild(std::shared_ptr<SceneNode> child);
        void                       removeChild(std::shared_ptr<SceneNode> child);
        void                       removeChild(const std::string& name);
        std::shared_ptr<SceneNode> getChild(const std::string& name) const;
        std::shared_ptr<SceneNode> getChild(size_t index) const;
        size_t                     getChildCount() const;
        std::shared_ptr<SceneNode> getParent() const;
        void                       setParent(std::shared_ptr<SceneNode> parent);
        void                       clearChildren();

        void forEachChild(std::function<void(std::shared_ptr<SceneNode>)> func);
        void forEachChild(std::function<void(const std::shared_ptr<SceneNode>&)> func) const;

    private:
        std::string                             m_name;
        entt::entity                            m_entity {entt::null};
        std::weak_ptr<SceneNode>                m_parent;
        std::vector<std::shared_ptr<SceneNode>> m_children;

        void updateParentReference(std::shared_ptr<SceneNode> new_parent);
    };

} // namespace RealmEngine
