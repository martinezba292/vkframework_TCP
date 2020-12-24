#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__ 1

#include "Components/component.h"
#include "vertex_buffer.h"

class VertexBuffer;
class Geometry : public Component {
public:
  Geometry();
  ~Geometry(){};
  Geometry(const Geometry&);

  void loadGeometry(glm::vec3* vertices, uint32 vertex_number, 
                    uint32* indices, uint32 index_number);
  void loadGeometry(VertexBuffer& buffer);
  void create();

private:
  VertexBuffer geometry_buffer_;

};

#endif // !__GEOMETRY_H__ 1

