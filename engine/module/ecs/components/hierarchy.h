#pragma once

#include <entt/entity/entity.hpp>
#include <vector>

namespace RealmEngine
{
    struct Parent
    {
        entt::entity entity {entt::null};
    };

    struct Children
    {
        std::vector<entt::entity> entities;
    };

} // namespace RealmEngine
