#pragma once

#include <memory>

namespace RealmEngine
{
    class Scene;

    class Editor
    {
    public:
        Editor() = default;
        ~Editor() noexcept;

        Editor(const Editor&)            = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&)                 = delete;
        Editor& operator=(Editor&&)      = delete;

        void initialize();
        void shutdown();

        void beginFrame() const;
        void endFrame() const;

        void setScene(std::shared_ptr<Scene> scene) { m_scene = scene; }

    private:
        bool                   m_initialized {false};
        std::shared_ptr<Scene> m_scene;
    };

} // namespace RealmEngine
