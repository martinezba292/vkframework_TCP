#ifndef __MATERIAL__
#define __MATERIAL__ 1

#include "common_def.h"

enum MaterialType {
  kMaterialType_NONE = -1,
  kMaterialType_UnlitColor = 0,
  kMaterialType_TextureSampler = 1,
  kMaterialType_MAX,
};

class Material {
public:
  Material();
  ~Material(){}
  Material(const Material&);

  int32 getMaterialId();
  int32 getMaterialType();
  void setMaterialType(MaterialType type);

private:
  int32 materialId_;
  MaterialType type_;
};

#endif // __MATERIAL__
