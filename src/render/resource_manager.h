#pragma once

namespace RealmEngine
{
    class GraphicResourceManager
    {
    public:
        GraphicResourceManager()           = default;
        ~GraphicResourceManager() noexcept = default;

        GraphicResourceManager(const GraphicResourceManager& that)            = delete;
        GraphicResourceManager(GraphicResourceManager&& that)                 = delete;
        GraphicResourceManager& operator=(const GraphicResourceManager& that) = delete;
        GraphicResourceManager& operator=(GraphicResourceManager&& that)      = delete;

        void initialize();
        void disposal();
    };

} // namespace RealmEngine