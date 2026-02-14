#include "command_executor.h"
#include "command.h"

namespace RealmEngine
{
    void CommandExecutor::execute(std::unique_ptr<ICommand> command)
    {
        if (command)
        {
            m_redo_stack.clear();
            RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
                m_undo_stack.emplace_back(std::move(u), std::move(r));
            };
            command->execute(reg);
        }
    }

    void CommandExecutor::execute(ICommand& command)
    {
        m_redo_stack.clear();
        RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
            m_undo_stack.emplace_back(std::move(u), std::move(r));
        };
        command.execute(reg);
    }

    void CommandExecutor::execute(ICommand&& command)
    {
        m_redo_stack.clear();
        RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
            m_undo_stack.emplace_back(std::move(u), std::move(r));
        };
        command.execute(reg);
    }

    void CommandExecutor::undo()
    {
        if (m_undo_stack.empty())
            return;
        auto [undo_fn, redo_fn] = std::move(m_undo_stack.back());
        m_undo_stack.pop_back();
        undo_fn();
        m_redo_stack.emplace_back(std::move(undo_fn), std::move(redo_fn));
    }

    void CommandExecutor::redo()
    {
        if (m_redo_stack.empty())
            return;
        auto [undo_fn, redo_fn] = std::move(m_redo_stack.back());
        m_redo_stack.pop_back();
        redo_fn();
        m_undo_stack.emplace_back(std::move(undo_fn), std::move(redo_fn));
    }

    bool CommandExecutor::canUndo() const { return !m_undo_stack.empty(); }

    bool CommandExecutor::canRedo() const { return !m_redo_stack.empty(); }

} // namespace RealmEngine
