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

    // Abstract single render pass. Subclasses implement setup / execute / teardown.
    class RenderPass
    {
    public:
        explicit RenderPass(const std::string& name) : m_name(name) {}
        virtual ~RenderPass() = default;

        // Called once after pipeline is built (compile shaders, create framebuffers, etc.)
        virtual void initialize(RHIDevice& device) = 0;

        // Called every frame
        virtual void execute(const RenderContext& ctx) = 0;

        // Cleanup
        virtual void dispose() = 0;

        const std::string& getName() const { return m_name; }

    protected:
        std::string m_name;
    };

} // namespace RealmEngine
