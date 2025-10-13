#pragma once

#include "gameplay/component.h"

namespace RealmEngine
{
    class MeshRenderer : public Component
    {
    public:
        MeshRenderer()  = default;
        ~MeshRenderer() = default;

        MeshRenderer(MeshRenderer&&)                 = default;
        MeshRenderer(const MeshRenderer&)            = delete;
        MeshRenderer& operator=(MeshRenderer&&)      = default;
        MeshRenderer& operator=(const MeshRenderer&) = delete;

    private:
    };
} // namespace RealmEngine