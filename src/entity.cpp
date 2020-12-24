#include "entity.h"

Entity::Entity()
{
  id_ = -1;

}

Entity::Entity(const Entity& other)
{
  id_ = other.id_;
  components_ = other.components_;
}

int32 Entity::getId()
{
  return id_;
}

uint32 Entity::addComponent(Component* new_component)
{
  auto itr = components_.find(new_component->getComponentType());
  if (itr != components_.end()) 
    return 1;

  components_.insert(std::make_pair(new_component->getComponentType(), new_component));
  return 0;
}

