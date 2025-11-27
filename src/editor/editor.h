#pragma once

#include <memory>
#include <vector>

namespace RealmEngine
{
    class Engine;
    class Widget;
    class EditorContext;

    class Editor
    {
    public:
        Editor();
        ~Editor() noexcept;

        Editor(const Editor&)            = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&)                 = delete;
        Editor& operator=(Editor&&)      = delete;

        void initialize();
        void shutdown();
        void run();

    private:
        void tick();

        void beginFrame() const;
        void render() const;
        void endFrame() const;

        bool                                 m_initialized {false};
        std::unique_ptr<Engine>              m_engine;
        std::shared_ptr<EditorContext>       m_context;
        std::vector<std::shared_ptr<Widget>> m_panels;
    };

} // namespace RealmEngine
