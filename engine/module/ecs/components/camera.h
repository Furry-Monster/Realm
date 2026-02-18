#pragma once

#include <cstdint>

namespace RealmEngine
{
    enum class CameraProjectionType : uint8_t
    {
        Perspective,
        Orthographic
    };

    struct Camera
    {
        CameraProjectionType projection_type {CameraProjectionType::Perspective};
        float                fov {45.0f};
        float                aspect_ratio {16.0f / 9.0f};
        float                near_plane {0.1f};
        float                far_plane {1000.0f};

        float ortho_left {-1.0f};
        float ortho_right {1.0f};
        float ortho_bottom {-1.0f};
        float ortho_top {1.0f};

        bool primary {true};
    };

} // namespace RealmEngine
