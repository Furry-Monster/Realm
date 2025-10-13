#pragma once

namespace RealmEngine
{
    class Scene
    {
    public:
        Scene()           = default;
        ~Scene() noexcept = default;

        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = default;
        Scene& operator=(Scene&&) noexcept = default;
    };
} // namespace RealmEngine