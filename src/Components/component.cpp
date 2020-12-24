#include "Components/component.h"

Component::Component()
{
  id_ = -1;
  type_ = ComponentType::kComponentType_NONE;
}

ComponentType Component::getComponentType()
{
  return type_;
}

int32 Component::getId()
{
  return id_;
}

