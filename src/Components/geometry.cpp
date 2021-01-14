#include "Components/geometry.h"
#include "resource_manager.h"
#include "dev/internal.h"

Geometry::Geometry()
{
  type_ = ComponentType::kComponentType_Geometry;
}

Geometry::Geometry(const Geometry& other)
{
  type_ = other.type_;
  geometry_buffer_ = other.geometry_buffer_;
}

void Geometry::loadGeometry(Vertex* vertices, uint32 vertex_number, uint32* indices, uint32 index_number)
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



void Geometry::update(ComponentUpdateData* buffer)
{
  buffer->drawCall.geometry = id_;
}

void Geometry::initWithPrimitive(PrimitiveType type)
{
  id_ = (int32)type;
}

