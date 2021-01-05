#ifndef __ENTITY_H__
#define __ENTITY_H__ 1

#include "Components/component.h"
#include <map>

class Material;
struct MaterialInfo {
  int32 id;
  int32 offset;
};

class Entity {
public:
  Entity();
  ~Entity();
  Entity(const Entity&);

  int32 getId();
  
  template <class T>
  T* getComponent(ComponentType type) {
    auto comp = components_.find(type);
    if (comp != components_.end()) 
      return static_cast<T*>(comp->second);

    return nullptr;
  }

  Material* getMaterial();
  int32 getMaterialOffset();

  uint32 addComponent(Component*);
  int8 setMaterial(Material*);

  void removeComponents();

private:
  int32 id_;
  MaterialInfo material_;
  std::map<ComponentType, Component*> components_;

  friend class ResourceManager;

};

#endif // __ENTITY_H__
