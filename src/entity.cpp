#include "entity.h"
#include "material.h"
#include "resource_manager.h"
#include "dev/internal.h"
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
  material_ = other.material_;
  components_ = other.components_;
}

Entity::~Entity()
{
  components_.clear();
}

int32 Entity::getId()
{
  return id_;
}


void Entity::updateEntity(UpdateData* buffer, uint64_t buffer_padding)
{
  Material* mat = getMaterial();
  
  for (auto& component : components_) {
    component.second->update(buffer);
  }
  buffer->drawCall.offset = material_.offset;
  if (mat) {
    buffer->drawCall.materialType = mat->getMaterialType();
    mat->updateMaterialSettings(buffer->model, material_.offset, buffer_padding);
  }
}

Material* Entity::getMaterial()
{
  if (material_.id < 0) return nullptr;

  return Scene::sceneMaterials[material_.id].get();
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

  PtrAlloc<Component> cmp = new_component;
  components_.insert(std::make_pair(new_component->getComponentType(), cmp));
  return 0;
}

int32 Entity::setMaterial(Material* material)
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

