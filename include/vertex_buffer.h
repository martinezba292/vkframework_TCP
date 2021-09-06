#ifndef __VERTEX_BUFFER__
#define __VERTEX_BUFFER__ 1

#include "common_def.h"
#include "glm/glm.hpp"


struct Vertex {
  glm::vec3 vertex;
  glm::vec3 normal;
  glm::vec2 uv;

  //Used for loadObj function in resource manager
  //Need to use Vertex struct with unordered map
  bool operator ==(const Vertex& other) const {
    return vertex == other.vertex && normal == other.normal && uv == other.uv;
  }
};

class VertexBuffer {
public:
  VertexBuffer();
  ~VertexBuffer();
  void loadVertices(Vertex* points, uint32 vertex_numb);
  void loadIndices(uint32* index, uint32 indices_number);
  const int32 getId();
  void copyBuffer(const VertexBuffer&);
  const Vertex* getVertexArray();
  uint32 getVertexNumber();

  static Vertex VertexInitializer(glm::vec3 vertex) {
    return { vertex, glm::vec3(0.0f), glm::vec2(0.0f) };
  }

  static Vertex VertexInitializer(glm::vec3 vertex, glm::vec3 normal) {
    return { vertex, normal, glm::vec2(0.0f) };
  }

private:
  int32 buffer_id_;
  std::vector<Vertex> vertex_array_;
  std::vector<uint32> indices_array_;

  friend class ResourceManager;
};


#endif
