#ifndef __ENTITY_H__
#define __ENTITY_H__ 1

#include "Components/component.h"
#include <map>

class Entity {
public:
  Entity();
  ~Entity(){}
  Entity(const Entity&);

  int32 getId();
  
  template <class T>
  T* getComponent(ComponentType type) {
    auto comp = components_.find(type);
    if (comp != components_.end()) 
      return static_cast<T*>(comp->second);

    return nullptr;
  }

  uint32 addComponent(Component*);

private:
  int32 id_;
  std::map<ComponentType, Component*> components_;

  friend class ResourceManager;

};

#endif // __ENTITY_H__
