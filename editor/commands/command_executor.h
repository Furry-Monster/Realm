#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace RealmEngine
{
    class ICommand;

    class CommandExecutor
    {
    public:
        static constexpr size_t DEFAULT_MAX_HISTORY = 200;

        explicit CommandExecutor(size_t max_history = DEFAULT_MAX_HISTORY);
        ~CommandExecutor() = default;

        CommandExecutor(const CommandExecutor&)            = delete;
        CommandExecutor& operator=(const CommandExecutor&) = delete;
        CommandExecutor(CommandExecutor&&)                 = delete;
        CommandExecutor& operator=(CommandExecutor&&)      = delete;

        void execute(std::unique_ptr<ICommand> command);
        void execute(ICommand& command);
        void execute(ICommand&& command);
        void undo();
        void redo();
        bool canUndo() const;
        bool canRedo() const;

        size_t getMaxHistory() const { return m_max_history; }
        void   setMaxHistory(size_t max_history) { m_max_history = max_history; }

    private:
        void executeImpl(ICommand& command);
        void trimHistory();

        using UndoRedoPair = std::pair<std::function<void()>, std::function<void()>>;
        std::vector<UndoRedoPair> m_undo_stack;
        std::vector<UndoRedoPair> m_redo_stack;
        size_t                    m_max_history;
    };

} // namespace RealmEngine
