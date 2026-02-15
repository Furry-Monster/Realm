#include "command_executor.h"
#include "command.h"

namespace RealmEngine
{
    CommandExecutor::CommandExecutor(size_t max_history) : m_max_history(max_history) {}

    void CommandExecutor::execute(std::unique_ptr<ICommand> command)
    {
        if (command)
        {
            m_redo_stack.clear();
            RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
                m_undo_stack.emplace_back(std::move(u), std::move(r));
            };
            command->execute(reg);
            trimHistory();
        }
    }

    void CommandExecutor::execute(ICommand& command)
    {
        m_redo_stack.clear();
        RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
            m_undo_stack.emplace_back(std::move(u), std::move(r));
        };
        command.execute(reg);
        trimHistory();
    }

    void CommandExecutor::execute(ICommand&& command)
    {
        m_redo_stack.clear();
        RegisterUndo reg = [this](std::function<void()> u, std::function<void()> r) {
            m_undo_stack.emplace_back(std::move(u), std::move(r));
        };
        command.execute(reg);
        trimHistory();
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

    void CommandExecutor::trimHistory()
    {
        if (m_max_history > 0 && m_undo_stack.size() > m_max_history)
        {
            size_t excess = m_undo_stack.size() - m_max_history;
            m_undo_stack.erase(m_undo_stack.begin(), m_undo_stack.begin() + static_cast<ptrdiff_t>(excess));
        }
    }

} // namespace RealmEngine
