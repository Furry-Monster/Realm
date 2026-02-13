#pragma once

#include <string>

namespace RealmEngine
{
    class RHIDevice;
    class RenderScene;
    class RenderCamera;

    // Per-frame data shared between all passes
    struct RenderContext
    {
        RHIDevice*    device {};
        RenderScene*  scene {};
        RenderCamera* camera {};
        int           viewport_width {0};
        int           viewport_height {0};
    };

    class RenderPass
    {
    public:
        explicit RenderPass(const std::string& name) : m_name(name) {}
        virtual ~RenderPass() = default;

        virtual void init(RHIDevice& device)           = 0;
        virtual void execute(const RenderContext& ctx) = 0;
        virtual void dispose()                         = 0;

        const std::string& getName() const { return m_name; }

    protected:
        std::string m_name;
    };

} // namespace RealmEngine
