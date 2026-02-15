#pragma once

namespace RealmEngine
{
    class RHIDevice;

    class FullscreenQuad
    {
    public:
        explicit FullscreenQuad(RHIDevice& device);
        ~FullscreenQuad();

        FullscreenQuad(const FullscreenQuad&)            = delete;
        FullscreenQuad& operator=(const FullscreenQuad&) = delete;

        void draw() const;

    private:
        struct Impl;
        Impl* m_impl {nullptr};
    };

} // namespace RealmEngine
