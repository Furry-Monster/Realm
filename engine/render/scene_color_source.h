#pragma once

namespace RealmEngine
{
    class RHIFramebuffer;

    /**
     * @brief
     * Abstraction over the pass that produces the main HDR scene color + depth.
     */
    class SceneColorSource
    {
    public:
        virtual ~SceneColorSource() = default;

        virtual RHIFramebuffer* getFramebuffer() const = 0;
    };

} // namespace RealmEngine
