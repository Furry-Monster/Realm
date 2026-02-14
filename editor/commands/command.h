#pragma once

namespace RealmEngine
{
    class ICommand
    {
    public:
        virtual ~ICommand()    = default;
        virtual void execute() = 0;
    };

} // namespace RealmEngine
