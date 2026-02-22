#pragma once

#include <memory>

#include "functional/render/ibl/ibl_geometry.h"

namespace RealmEngine
{
    class RHIDevice;
    class RHITexture;

    class Skybox
    {
    public:
        explicit Skybox(RHIDevice& device, RHITexture* env_cubemap);

        void draw(RHIDevice& device) const;

    private:
        RHITexture*                  m_env_cubemap {nullptr};
        std::unique_ptr<IblCubeMesh> m_cube;
    };

} // namespace RealmEngine
