#include "resource_manager.h"
#include "vertex_buffer.h"
#include "internal.h"
#include <iterator>

ResourceManager* ResourceManager::instance_ = nullptr;


ResourceManager::~ResourceManager()
{
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


ResourceManager::ResourceManager()
{
  resources_ = new Resources();
}

