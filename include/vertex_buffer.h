#ifndef __VERTEX_BUFFER__
#define __VERTEX_BUFFER__ 1

#include "common_def.h"
#include "glm/glm.hpp"

//struct Vertex {
//  glm::vec3 point_;
//};

class VertexBuffer {
public:
  VertexBuffer();
  ~VertexBuffer();
  void loadVertices(glm::vec3* points, uint32 vertex_numb);
  void loadIndices(uint32* index, uint32 indices_number);
  const int32 getId();
  void copyBuffer(const VertexBuffer&);
  const glm::vec3* getVertexArray();
  uint32 getVertexNumber();


private:
  int32 buffer_id_;
  std::vector<glm::vec3> vertex_array_;
  std::vector<uint32> indices_array_;

  friend class ResourceManager;
};


#endif
