#ifndef __COMPONENT_H__
#define __COMPONENT_H__ 1

#include "common_def.h"
#include "dev/referenced.h"

enum class ComponentType {
  kComponentType_NONE = 0,
  kComponentType_Transform = 1,
  kComponentType_Geometry = 2,
  kComponentType_Light,
  kComponentType_MAX
};

struct ComponentUpdateData;
class Component : public Referenced {
public:
  ComponentType getComponentType();
  int32 getId();
  virtual void update(ComponentUpdateData*) = 0;

protected:
  Component();
  virtual ~Component(){}
  ComponentType type_;
  int32 id_;

private:
  Component(const Component& other);
};

#endif // __COMPONENT_H__
