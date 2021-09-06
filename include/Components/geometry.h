#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__ 1

#include "Components/component.h"
#include "vertex_buffer.h"
#include <string>

enum class PrimitiveType {
  kPrimitiveType_NONE = -1,
  kPrimitiveType_Triangle = 0,
  kPrimitiveType_Quad = 1,
  kPrimitiveType_Cube,
  kPrimitiveType_Sphere,
  kPrimitiveType_Terrain,
};

class Geometry : public Component {
public:
  Geometry();

  void loadGeometry(Vertex* vertices, uint32 vertex_number, 
                    uint32* indices, uint32 index_number);
  void loadGeometry(VertexBuffer& buffer);
  void create();
  void update(UpdateData*) override;
  void initWithPrimitive(PrimitiveType type);

protected:
  virtual ~Geometry(){};

private:
  Geometry(const Geometry&);
  VertexBuffer geometry_buffer_;

};

#endif // !__GEOMETRY_H__ 1

