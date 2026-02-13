#pragma once

#include <entt/entity/entity.hpp>
#include <vector>

namespace RealmEngine
{
    // Parent-child relationship components.
    // Managed by HierarchySystem -- do not modify directly.

    struct Parent
    {
        entt::entity entity {entt::null};
    };

    struct Children
    {
        std::vector<entt::entity> entities;
    };

} // namespace RealmEngine
