#pragma once

#include <functional>

namespace RealmEngine
{
    using RegisterUndo = std::function<void(std::function<void()> undo, std::function<void()> redo)>;

    class ICommand
    {
    public:
        virtual ~ICommand() noexcept = default;
        virtual void execute(RegisterUndo registerUndo = nullptr) = 0;
    };

} // namespace RealmEngine
