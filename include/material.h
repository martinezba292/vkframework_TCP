#ifndef __MATERIAL__
#define __MATERIAL__ 1

#include "common_def.h"
#include "glm/glm.hpp"

enum MaterialType {
  kMaterialType_NONE = -1,
  kMaterialType_UnlitColor = 0,
  kMaterialType_TextureSampler = 1,
  kMaterialType_MAX,
};

class Texture;
union UniformBlocks;

class Material {
public:
  Material();
  ~Material();
  Material(const Material&);

  int32 getMaterialId();
  int32 getMaterialType();
  void setMaterialType(MaterialType type);
  int32 setMaterialColor(glm::vec3 color);
  int32 setMaterialTexture(Texture texture);

  UniformBlocks& getMaterialSettings();

private:
  int32 materialId_;
  MaterialType type_;
  UniformBlocks* settings_;

  friend class ResourceManager;
};

#endif // __MATERIAL__
