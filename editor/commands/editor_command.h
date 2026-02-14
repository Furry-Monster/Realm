#pragma once

#include <memory>

namespace RealmEngine
{
    class EditorCommand
    {
    public:
        virtual ~EditorCommand() = default;
        virtual void execute()   = 0;
    };

} // namespace RealmEngine
