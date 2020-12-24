#include "Components/geometry.h"
#include "vertex_buffer.h"
#include "resource_manager.h"

Geometry::Geometry()
{
  type_ = kComponentType_Geometry;
}

void Geometry::loadGeometry(glm::vec3* vertices, uint32 vertex_number, uint32* indices, uint32 index_number)
{
  geometry_buffer_.loadVertices(vertices, vertex_number);
  geometry_buffer_.loadIndices(indices, index_number);
}

void Geometry::loadGeometry(VertexBuffer& buffer)
{
  geometry_buffer_.copyBuffer(buffer);
}

void Geometry::create()
{
  ResourceManager::Get()->createVertexBuffer(&geometry_buffer_);
  id_ = geometry_buffer_.getId();
}

