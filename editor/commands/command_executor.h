#pragma once

#include <memory>

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
    };

} // namespace RealmEngine
