#ifndef __COMPONENT_H__
#define __COMPONENT_H__ 1

#include "common_def.h"

enum ComponentType : uint8 {
  kComponentType_NONE = 0,
  kComponentType_Transform = 1,
  kComponentType_Geometry = 2,
  kComponentType_MAX
};


class Component {
public:
  Component();
  ~Component(){}
  Component(const Component&){}
  ComponentType getComponentType();
  int32 getId();

protected:
  enum ComponentType type_;
  int32 id_;
};

#endif // __COMPONENT_H__
