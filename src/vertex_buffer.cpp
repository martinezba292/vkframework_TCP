#include "vertex_buffer.h"

VertexBuffer::VertexBuffer()
{
  buffer_id_ = -1;
  vertex_array_.clear();
}

VertexBuffer::~VertexBuffer()
{
  vertex_array_.clear();
}

void VertexBuffer::loadPoints(glm::vec3* points, uint32 vertex_numb)
{
  vertex_array_.clear();
  vertex_array_.resize(vertex_numb);
  std::copy(points, points + vertex_numb, vertex_array_.begin());
}

void VertexBuffer::loadIndices(uint32* index, uint32 indices_number)
{
  indices_array_.clear();
  indices_array_.resize(indices_number);
  std::copy(index, index + indices_number, indices_array_.begin());
}

const int32 VertexBuffer::getId()
{
  return buffer_id_;
}

const glm::vec3* VertexBuffer::getVertexArray()
{
  return vertex_array_.data();
}

uint32 VertexBuffer::getVertexNumber()
{
  return vertex_array_.size();
}

