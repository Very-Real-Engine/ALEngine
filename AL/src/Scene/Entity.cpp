#include "Scene/Entity.h"

namespace ale
{
Entity::Entity(entt::entity handle, Scene *scene) : m_EntityHandle(handle), m_Scene(scene)
{
}
} // namespace ale