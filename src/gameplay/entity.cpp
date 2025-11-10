#include "gameplay/entity.h"

namespace RealmEngine
{
    Entity::Entity(std::shared_ptr<RenderModel> model) : mModel(model) {}

    void Entity::setPosition(glm::vec3 position) { mPosition = position; }

    glm::vec3 Entity::getPosition() const { return mPosition; }

    void Entity::setScale(glm::vec3 scale) { mScale = scale; }

    glm::vec3 Entity::getScale() const { return mScale; }

    void Entity::setOrientation(glm::quat orientation) { mOrientation = orientation; }

    glm::quat Entity::getOrientation() const { return mOrientation; }

    std::shared_ptr<RenderModel> Entity::getModel() const { return mModel; }
} // namespace RealmEngine
