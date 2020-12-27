#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__ 1

#include "Components/component.h"
#include "vertex_buffer.h"

class Geometry : public Component {
public:
  Geometry();
  ~Geometry(){};
  Geometry(const Geometry&);

  void loadGeometry(Vertex* vertices, uint32 vertex_number, 
                    uint32* indices, uint32 index_number);
  void loadGeometry(VertexBuffer& buffer);
  void create();

private:
  VertexBuffer geometry_buffer_;

};

struct PrimitiveGeometry {
  static Geometry triangle;
  static Geometry cube;
};

#endif // !__GEOMETRY_H__ 1

