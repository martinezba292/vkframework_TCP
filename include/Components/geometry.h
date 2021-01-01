#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__ 1

#include "Components/component.h"
#include "vertex_buffer.h"

enum PrimitiveType {
  kPrimitiveType_NONE = -1,
  kPrimitiveType_Triangle = 0,
  kPrimitiveType_Quad = 1,
  kPrimitiveType_Cube,
  kPrimitiveType_Sphere
};

class Geometry : public Component {
public:
  Geometry();
  ~Geometry(){};
  Geometry(const Geometry&);

  void loadGeometry(Vertex* vertices, uint32 vertex_number, 
                    uint32* indices, uint32 index_number);
  void loadGeometry(VertexBuffer& buffer);
  void create();

  void initWithPrimitive(PrimitiveType type);

private:
  VertexBuffer geometry_buffer_;

};

struct PrimitiveGeometry {
  static Geometry triangle;
  static Geometry quad;
  static Geometry cube;
  static Geometry sphere;
};

#endif // !__GEOMETRY_H__ 1

