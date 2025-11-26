#include "gameplay/scene/scene_node.h"
#include <algorithm>
#include <functional>

namespace RealmEngine
{
    SceneNode::SceneNode() : m_name("SceneNode"), m_entity_id(0) {}
    SceneNode::SceneNode(const std::string& name) : m_name(name), m_entity_id(0) {}

    void               SceneNode::setName(const std::string& name) { m_name = name; }
    const std::string& SceneNode::getName() const { return m_name; }

    void   SceneNode::setEntityId(size_t entity_id) { m_entity_id = entity_id; }
    size_t SceneNode::getEntityId() const { return m_entity_id; }
    bool   SceneNode::hasEntity() const { return m_entity_id != 0; }

    void SceneNode::addChild(std::shared_ptr<SceneNode> child)
    {
        if (!child)
            return;

        auto old_parent = child->m_parent.lock();
        if (old_parent)
        {
            old_parent->removeChild(child);
        }

        m_children.push_back(child);
        child->updateParentReference(shared_from_this());
    }

    void SceneNode::removeChild(std::shared_ptr<SceneNode> child)
    {
        if (!child)
            return;

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end())
        {
            (*it)->updateParentReference(nullptr);
            m_children.erase(it);
        }
    }

    void SceneNode::removeChild(const std::string& name)
    {
        auto it = std::find_if(m_children.begin(), m_children.end(), [&name](const std::shared_ptr<SceneNode>& node) {
            return node->getName() == name;
        });

        if (it != m_children.end())
        {
            (*it)->updateParentReference(nullptr);
            m_children.erase(it);
        }
    }

    std::shared_ptr<SceneNode> SceneNode::getChild(const std::string& name) const
    {
        auto it = std::find_if(m_children.begin(), m_children.end(), [&name](const std::shared_ptr<SceneNode>& node) {
            return node->getName() == name;
        });

        return (it != m_children.end()) ? *it : nullptr;
    }

    std::shared_ptr<SceneNode> SceneNode::getChild(size_t index) const
    {
        return (index < m_children.size()) ? m_children[index] : nullptr;
    }

    size_t SceneNode::getChildCount() const { return m_children.size(); }

    std::shared_ptr<SceneNode> SceneNode::getParent() const { return m_parent.lock(); }

    void SceneNode::setParent(std::shared_ptr<SceneNode> parent)
    {
        if (parent)
        {
            parent->addChild(shared_from_this());
        }
        else
        {
            auto current_parent = m_parent.lock();
            if (current_parent)
                current_parent->removeChild(shared_from_this());
        }
    }

    void SceneNode::forEachChild(std::function<void(std::shared_ptr<SceneNode>)> func)
    {
        for (auto& child : m_children)
            func(child);
    }

    void SceneNode::forEachChild(std::function<void(const std::shared_ptr<SceneNode>&)> func) const
    {
        for (const auto& child : m_children)
            func(child);
    }

    void SceneNode::clearChildren()
    {
        for (auto& child : m_children)
            child->updateParentReference(nullptr);

        m_children.clear();
    }

    void SceneNode::updateParentReference(std::shared_ptr<SceneNode> new_parent)
    {
        if (new_parent)
            m_parent = new_parent;
        else
            m_parent.reset();
    }

} // namespace RealmEngine
