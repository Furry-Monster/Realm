#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace RealmEngine
{
    class ICommand;

    class CommandExecutor
    {
    public:
        CommandExecutor()  = default;
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

    private:
        using UndoRedoPair = std::pair<std::function<void()>, std::function<void()>>;
        std::vector<UndoRedoPair> m_undo_stack;
        std::vector<UndoRedoPair> m_redo_stack;
    };

} // namespace RealmEngine
