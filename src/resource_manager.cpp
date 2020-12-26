#include "resource_manager.h"
#include "vertex_buffer.h"
#include "internal.h"
#include <iterator>
#include "entity.h"

ResourceManager* ResourceManager::instance_ = nullptr;
std::vector<Entity> Scene::sceneEntities;
std::chrono::steady_clock::time_point Scene::lastTime;

ResourceManager::~ResourceManager()
{
  for (auto& entity : Scene::sceneEntities) {
    for (auto component : entity.components_) {
      delete(component.second);
    }
  }
  delete(resources_);
}

ResourceManager* ResourceManager::Get()
{
   if (!instance_) {
     instance_ = new ResourceManager();
   }
 
   return instance_;
}


Resources* ResourceManager::getResources() const
{
  return resources_;
}

void ResourceManager::createVertexBuffer(VertexBuffer* buffer)
{
  buffer->buffer_id_ = resources_->vertex_data.size();
  InternalVertexData newVertexData;
  newVertexData.vertex.resize(buffer->getVertexNumber());

  const glm::vec3* vertices = buffer->getVertexArray();
  std::copy(vertices,
            vertices + buffer->getVertexNumber(),
            newVertexData.vertex.data());

  uint32 indices_number = buffer->indices_array_.size();
  newVertexData.indices.resize(indices_number);
  const uint32* indices = buffer->indices_array_.data();
  std::copy(indices, indices + indices_number, newVertexData.indices.data());

  resources_->vertex_data.push_back(newVertexData);
}


void ResourceManager::createEntity(Entity* new_entity)
{
  if (new_entity == nullptr) return;

  new_entity->id_ = Scene::sceneEntities.size();
  Scene::sceneEntities.push_back(*new_entity);
}

ResourceManager::ResourceManager()
{
  resources_ = new Resources();
}

