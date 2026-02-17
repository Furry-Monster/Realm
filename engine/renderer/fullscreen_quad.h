#pragma once

#include <memory>

namespace RealmEngine
{
    class RHIDevice;

    class FullscreenQuad
    {
    public:
        explicit FullscreenQuad(RHIDevice& device);
        ~FullscreenQuad();

        FullscreenQuad(const FullscreenQuad&)            = delete;
        FullscreenQuad& operator=(const FullscreenQuad&)  = delete;
        FullscreenQuad(FullscreenQuad&&)                 = delete;
        FullscreenQuad& operator=(FullscreenQuad&&)      = delete;

        void draw() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace RealmEngine
