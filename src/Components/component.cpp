#include "Components/component.h"

Component::Component()
{
  id_ = -1;
  type_ = ComponentType::kComponentType_NONE;
}

Component::Component(const Component& other)
{
  type_ = other.type_;
  id_ = other.id_;
}

ComponentType Component::getComponentType()
{
  return type_;
}

int32 Component::getId()
{
  return id_;
}


