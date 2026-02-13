#pragma once

// Legacy compatibility header.
// The concrete shader implementation now lives in rhi/opengl/gl_shader.
// All new code should use RHIShader directly.

#include "rhi/rhi_shader.h"

namespace RealmEngine
{
    // Alias so existing draw(Shader&) call sites compile without mass-rename.
    using Shader = RHIShader;

} // namespace RealmEngine
