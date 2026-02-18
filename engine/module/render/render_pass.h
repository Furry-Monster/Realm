#pragma once

#include <string>

#include "module/render/viewport_display_mode.h"

namespace RealmEngine
{
    class RHIDevice;
    class RenderScene;
    class RenderCamera;

    class RHIFramebuffer;

    struct RenderContext
    {
        RHIDevice*          device {};
        RenderScene*        scene {};
        RenderCamera*       camera {};
        int                 viewport_width {0};
        int                 viewport_height {0};
        ViewportDisplayMode display_mode {ViewportDisplayMode::Lit};
        RHIFramebuffer*     viewport_framebuffer {nullptr};
    };

    /**
     * @Lifecycle:
     * init (one-time resource setup) -> execute (per-frame) -> dispose (teardown).
     */
    class RenderPass
    {
    public:
        explicit RenderPass(const std::string& name) : m_name(name) {}
        virtual ~RenderPass() noexcept = default;

        virtual void init(RHIDevice& device)           = 0;
        virtual void execute(const RenderContext& ctx) = 0;
        virtual void dispose()                         = 0;

        const std::string& getName() const { return m_name; }

    protected:
        std::string m_name;
    };

} // namespace RealmEngine
