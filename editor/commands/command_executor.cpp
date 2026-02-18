#include "command_executor.h"
#include "command.h"

namespace RealmEngine
{
    CommandExecutor::CommandExecutor(const size_t max_history) : m_max_history(max_history) {}

    void CommandExecutor::execute(const std::unique_ptr<ICommand>& command)
    {
        if (!command)
            return;
        executeImpl(*command);
    }

    void CommandExecutor::execute(ICommand& command) { executeImpl(command); }

    void CommandExecutor::execute(ICommand&& command) { executeImpl(command); }

    void CommandExecutor::executeImpl(ICommand& command)
    {
        const size_t       prev_size = m_undo_stack.size();
        const RegisterUndo reg       = [this](std::function<void()> u, std::function<void()> r) {
            m_undo_stack.emplace_back(std::move(u), std::move(r));
        };
        try
        {
            command.execute(reg);
        }
        catch (...)
        {
            // Roll back any undo entries the command may have registered
            m_undo_stack.resize(prev_size);
            throw;
        }
        // Only clear redo stack after successful execution
        m_redo_stack.clear();
        trimHistory();
    }

    void CommandExecutor::undo()
    {
        if (m_undo_stack.empty())
            return;
        auto entry = std::move(m_undo_stack.back());
        m_undo_stack.pop_back();
        try
        {
            entry.first();
        }
        catch (...)
        {
            // Push back to redo even on exception so the pair is not lost
            m_redo_stack.emplace_back(std::move(entry));
            throw;
        }
        m_redo_stack.emplace_back(std::move(entry));
    }

    void CommandExecutor::redo()
    {
        if (m_redo_stack.empty())
            return;
        auto entry = std::move(m_redo_stack.back());
        m_redo_stack.pop_back();
        try
        {
            entry.second();
        }
        catch (...)
        {
            m_undo_stack.emplace_back(std::move(entry));
            throw;
        }
        m_undo_stack.emplace_back(std::move(entry));
    }

    bool CommandExecutor::canUndo() const { return !m_undo_stack.empty(); }

    bool CommandExecutor::canRedo() const { return !m_redo_stack.empty(); }

    void CommandExecutor::trimHistory()
    {
        if (m_max_history > 0 && m_undo_stack.size() > m_max_history)
        {
            const size_t excess = m_undo_stack.size() - m_max_history;
            m_undo_stack.erase(m_undo_stack.begin(), m_undo_stack.begin() + static_cast<ptrdiff_t>(excess));
        }
    }

} // namespace RealmEngine
