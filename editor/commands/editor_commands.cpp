#include "editor_commands.h"

#include "bridge/editor_engine_bridge.h"
#include "editor_context.h"
#include "module/scene/scene.h"
#include "module/scene/scene_node.h"
#include "module/scene/scene_serializer.h"
#include "panels/file_dialog_widget.h"
#include "widget.h"

#include <filesystem>

#include "core/base/macros.h"

namespace RealmEngine
{
    NewSceneCommand::NewSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void NewSceneCommand::execute(RegisterUndo)
    {
        const auto new_scene = m_bridge->createDefaultScene();
        if (new_scene)
        {
            m_bridge->setCurrentScene(new_scene);
            RE_LOG_INFO("New scene created");
        }
    }

    OpenSceneCommand::OpenSceneCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog) :
        m_bridge(&bridge), m_file_dialog(file_dialog)
    {}

    void OpenSceneCommand::execute(RegisterUndo)
    {
        if (m_file_dialog)
        {
            const std::filesystem::path initial_path = m_bridge->getConfigRootFolder();
            m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
        }
    }

    SaveSceneCommand::SaveSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void SaveSceneCommand::execute(RegisterUndo)
    {
        if (!m_bridge->getCurrentScene())
        {
            RE_LOG_INFO("No scene to save");
            return;
        }
        std::filesystem::path scene_file = m_bridge->getSceneFileFromConfig();
        if (m_bridge->saveCurrentScene(scene_file.string()))
            RE_LOG_INFO("Scene saved to: " + scene_file.string());
        else
            RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
    }

    SaveSceneAsCommand::SaveSceneAsCommand(EditorEngineBridge& bridge, FileDialogWidget* file_dialog) :
        m_bridge(&bridge), m_file_dialog(file_dialog)
    {}

    void SaveSceneAsCommand::execute(RegisterUndo)
    {
        if (m_file_dialog && m_bridge->getCurrentScene())
        {
            const std::filesystem::path initial_path = m_bridge->getConfigRootFolder();
            m_file_dialog->open(FileDialogWidget::Mode::Save, "Save Scene As", ".json", initial_path);
        }
    }

    ReloadSceneCommand::ReloadSceneCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void ReloadSceneCommand::execute(RegisterUndo)
    {
        if (!m_bridge->getCurrentScene())
            return;

        std::filesystem::path scene_file = m_bridge->getSceneFileFromConfig();
        if (!std::filesystem::exists(scene_file))
        {
            RE_LOG_WARN("Scene file not found: " + scene_file.string());
            return;
        }

        const auto loaded = m_bridge->loadScene(scene_file.string());
        if (loaded)
        {
            m_bridge->setCurrentScene(loaded);
            m_bridge->initializeCameraForScene(loaded);
            RE_LOG_INFO("Scene reloaded from: " + scene_file.string());
        }
        else
        {
            RE_LOG_ERROR("Failed to reload scene from: " + scene_file.string());
        }
    }

    ExitCommand::ExitCommand(EditorEngineBridge& bridge) : m_bridge(&bridge) {}

    void ExitCommand::execute(RegisterUndo) { m_bridge->requestWindowClose(); }

    TogglePanelCommand::TogglePanelCommand(std::vector<std::shared_ptr<Widget>>& widgets, const size_t index) :
        m_widgets(widgets), m_index(index)
    {}

    void TogglePanelCommand::execute(RegisterUndo)
    {
        if (m_index < m_widgets.size())
        {
            const auto& w = m_widgets[m_index];
            if (w)
                w->setOpen(!w->isOpen());
        }
    }

    namespace
    {
        void destroyNodeAndSubtree(Scene& scene, const std::shared_ptr<SceneNode>& node)
        {
            if (!node)
                return;
            std::vector<std::shared_ptr<SceneNode>> children;
            node->forEachChild([&children](const std::shared_ptr<SceneNode>& c) { children.push_back(c); });
            for (auto& child : children)
                destroyNodeAndSubtree(scene, child);
            if (node->hasEntity() && scene.valid(node->getEntity()))
                scene.destroyEntity(node->getEntity());
            const auto parent = node->getParent();
            if (parent)
                parent->removeChild(node);
        }
        void removeNodeWithUndo(EditorEngineBridge* bridge, EditorContext* context, RegisterUndo registerUndo)
        {
            const auto scene = bridge->getCurrentScene();
            const auto node  = context->getSelectedNode();
            if (!scene || !node)
                return;
            const auto root = scene->getRoot();
            if (node == root)
                return;
            const auto parent = node->getParent();
            if (!parent)
                return;
            std::string json = SceneSerializer::serializeNodeToJson(node, *scene);
            destroyNodeAndSubtree(*scene, node);
            context->clearSelectedNode();
            context->clearSelectedEntity();
            scene->markDirty();
            if (registerUndo)
            {
                auto pasted_storage = std::make_shared<std::shared_ptr<SceneNode>>();
                auto parent_weak    = std::weak_ptr<SceneNode>(parent);
                registerUndo(
                    [pasted_storage, bridge, context, json, parent_weak] {
                        auto p = parent_weak.lock();
                        if (!p)
                            return;
                        const auto pasted = bridge->pasteEntityFromClipboard(json, p);
                        if (pasted)
                        {
                            *pasted_storage = pasted;
                            context->setSelectedNode(pasted);
                            if (pasted->hasEntity())
                                context->setSelectedEntity(pasted->getEntity());
                        }
                    },
                    [pasted_storage, bridge, context] {
                        if (!*pasted_storage)
                            return;
                        const auto sc = bridge->getCurrentScene();
                        if (!sc)
                            return;
                        destroyNodeAndSubtree(*sc, *pasted_storage);
                        context->clearSelectedNode();
                        context->clearSelectedEntity();
                        *pasted_storage = nullptr;
                        sc->markDirty();
                    });
            }
        }
    } // namespace

    DeleteEntityCommand::DeleteEntityCommand(EditorEngineBridge& bridge, EditorContext& context) :
        m_bridge(&bridge), m_context(&context)
    {}

    void DeleteEntityCommand::execute(const RegisterUndo registerUndo)
    {
        removeNodeWithUndo(m_bridge, m_context, registerUndo);
    }

    CopyEntityCommand::CopyEntityCommand(EditorEngineBridge& bridge, EditorContext& context) :
        m_bridge(&bridge), m_context(&context)
    {}

    void CopyEntityCommand::execute(RegisterUndo)
    {
        const auto scene = m_bridge->getCurrentScene();
        const auto node  = m_context->getSelectedNode();
        if (!scene || !node)
            return;
        const std::string json = SceneSerializer::serializeNodeToJson(node, *scene);
        m_context->setEntityClipboard(json);
    }

    PasteEntityCommand::PasteEntityCommand(EditorEngineBridge& bridge, EditorContext& context) :
        m_bridge(&bridge), m_context(&context)
    {}

    void PasteEntityCommand::execute(RegisterUndo registerUndo)
    {
        if (!m_context->hasEntityClipboard())
            return;
        const auto scene = m_bridge->getCurrentScene();
        if (!scene)
            return;
        const auto parent = m_context->hasSelectedNode() ? m_context->getSelectedNode() : scene->getRoot();
        if (!parent)
            return;
        std::string json   = m_context->getEntityClipboard();
        auto        pasted = m_bridge->pasteEntityFromClipboard(json, parent);
        if (pasted && pasted->hasEntity() && registerUndo)
        {
            m_context->setSelectedNode(pasted);
            m_context->setSelectedEntity(pasted->getEntity());
            auto  pasted_storage = std::make_shared<std::shared_ptr<SceneNode>>(pasted);
            auto* bridge         = m_bridge;
            auto* context        = m_context;
            auto  parent_weak    = std::weak_ptr<SceneNode>(parent);
            registerUndo(
                [pasted_storage, bridge, context] {
                    if (!*pasted_storage)
                        return;
                    const auto sc = bridge->getCurrentScene();
                    if (!sc)
                        return;
                    destroyNodeAndSubtree(*sc, *pasted_storage);
                    if (context->getSelectedNode() == *pasted_storage)
                    {
                        context->clearSelectedNode();
                        context->clearSelectedEntity();
                    }
                    *pasted_storage = nullptr;
                    sc->markDirty();
                },
                [pasted_storage, bridge, context, json, parent_weak] {
                    const auto p = parent_weak.lock();
                    if (!p)
                        return;

                    const auto restored = bridge->pasteEntityFromClipboard(json, p);
                    if (restored)
                    {
                        *pasted_storage = restored;
                        context->setSelectedNode(restored);
                        if (restored->hasEntity())
                            context->setSelectedEntity(restored->getEntity());
                    }
                });
        }
    }

    CutEntityCommand::CutEntityCommand(EditorEngineBridge& bridge, EditorContext& context) :
        m_bridge(&bridge), m_context(&context)
    {}

    void CutEntityCommand::execute(const RegisterUndo registerUndo)
    {
        CopyEntityCommand(*m_bridge, *m_context).execute(nullptr);
        removeNodeWithUndo(m_bridge, m_context, registerUndo);
    }

    DuplicateEntityCommand::DuplicateEntityCommand(EditorEngineBridge& bridge, EditorContext& context) :
        m_bridge(&bridge), m_context(&context)
    {}

    void DuplicateEntityCommand::execute(const RegisterUndo registerUndo)
    {
        // Preserve existing clipboard so Duplicate doesn't clobber it
        const std::string saved_clipboard = m_context->getEntityClipboard();
        CopyEntityCommand(*m_bridge, *m_context).execute(nullptr);
        PasteEntityCommand(*m_bridge, *m_context).execute(registerUndo);
        m_context->setEntityClipboard(saved_clipboard);
    }

} // namespace RealmEngine
