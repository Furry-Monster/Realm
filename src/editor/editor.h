#pragma once

#include <memory>

namespace RealmEngine
{
    class Engine;

    class Editor
    {
    public:
        Editor()           = default;
        ~Editor() noexcept = default;

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

        bool                    m_initialized {false};
        std::unique_ptr<Engine> m_engine;
    };

} // namespace RealmEngine
