#include "resource_manager.h"
#include "vertex_buffer.h"
#include "internal.h"
#include <iterator>
#include "entity.h"
#include "Components/geometry.h"

Geometry PrimitiveGeometry::triangle;
Geometry PrimitiveGeometry::cube;

ResourceManager* ResourceManager::instance_ = nullptr;
Camera Scene::camera;
std::vector<Entity> Scene::sceneEntities;
std::chrono::steady_clock::time_point Scene::lastTime;


ResourceManager::~ResourceManager()
{
  for (auto& entity : Scene::sceneEntities) {
    for (auto component : entity.components_) {
      delete(component.second);
    }
  }
  _aligned_free(resources_->dynamicUniformData);
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

void ResourceManager::initPrimitiveGeometries()
{
  Vertex triangle[] = {
    VertexBuffer::VertexInitializer({0.5f, 0.5f, -2.0f}),
    VertexBuffer::VertexInitializer({0.0f, -0.5f,-2.0f}),
    VertexBuffer::VertexInitializer({-0.5f,0.5f, -2.0f})
  };

  uint32 triangle_indices[] = { 0, 1, 2 };

  PrimitiveGeometry::triangle.loadGeometry(triangle, 3, triangle_indices, 3);

  Vertex cube[] = {
    ///FRONT
    VertexBuffer::VertexInitializer({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),

    ///REAR
    VertexBuffer::VertexInitializer({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),

    ///RIGHT
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),

    ///LEFT
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}),

    ///DOWN
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),

    ///UP
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f})
  };

  uint32 cube_indices[] = { 0,  2,  1,  2,  0,  3,
                            4,  5,  6,  4,  6,  7,
                            8,  10, 9,  8,  9,  11,
                            12, 14, 13, 12, 15, 14,
                            16, 17, 19, 19, 17, 18,
                            23, 21, 20, 21, 23, 22 };

  PrimitiveGeometry::cube.loadGeometry(cube, 24, cube_indices, 36);
}

void ResourceManager::createVertexBuffer(VertexBuffer* buffer)
{
  buffer->buffer_id_ = resources_->vertex_data.size();
  InternalVertexData newVertexData;
  newVertexData.vertex.resize(buffer->getVertexNumber());

  const Vertex* vertices = buffer->getVertexArray();
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

