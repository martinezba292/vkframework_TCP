#include "entity.h"
#include "material.h"
#include "resource_manager.h"
#include "internal.h"
#include <stdexcept>

Entity::Entity()
{
  id_ = -1;
  material_.offset = -1;
  material_.id = -1;
}

Entity::Entity(const Entity& other)
{
  id_ = other.id_;
  components_ = other.components_;
}

Entity::~Entity()
{
  //removeComponents();
}

int32 Entity::getId()
{
  return id_;
}


Material* Entity::getMaterial()
{
  if (material_.id < 0) return nullptr;

  return &Scene::sceneMaterials[material_.id];
}


int32 Entity::getMaterialOffset()
{
  return material_.offset;
}


uint32 Entity::addComponent(Component* new_component)
{
  auto itr = components_.find(new_component->getComponentType());
  if (itr != components_.end()) 
    return 1;

  components_.insert(std::make_pair(new_component->getComponentType(), new_component));
  return 0;
}

int8 Entity::setMaterial(Material* material)
{
  if (material_.id >= 0) return -1;
  if (material->getMaterialId() < 0)
    throw std::runtime_error("Material hasn't been created");

  InternalMaterial* mat = &ResourceManager::Get()->getResources()->internalMaterials[material->getMaterialType()];

  material_.id = material->getMaterialId();
  material_.offset = mat->entitiesReferenced;
  ++mat->entitiesReferenced;

  return 0;
}

void Entity::removeComponents()
{
  for (auto& component : components_) {
    delete(component.second);
  }

  components_.clear();
}

