#include "command_executor.h"
#include "command.h"

namespace RealmEngine
{
    void CommandExecutor::execute(std::unique_ptr<ICommand> command)
    {
        if (command)
            command->execute();
    }

    void CommandExecutor::execute(ICommand& command) { command.execute(); }

    void CommandExecutor::execute(ICommand&& command) { command.execute(); }

} // namespace RealmEngine
