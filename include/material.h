#ifndef __MATERIAL__
#define __MATERIAL__ 1

#include "common_def.h"
#include "glm/glm.hpp"
#include "dev/referenced.h"


enum class MaterialType {
  kMaterialType_NONE = -1,
  kMaterialType_UnlitColor = 0,
  kMaterialType_BasicPBR = 1,
  kMaterialType_PBRIBL,
  kMaterialType_TextureSampler,
  kMaterialType_Skybox,
  //kMaterialType_EquirectangularCube,
  kMaterialType_MAX,
};

class Camera;
class Texture;
union UniformBlocks;
struct ComponentUpdateData;
class Material : public Referenced {
public:
  Material();

  int32 getMaterialId();
  int32 getMaterialType();
  void setMaterialType(MaterialType type);
  int32 setMaterialColor(glm::vec3 color);
  int32 setMaterialTexture(Texture& texture);
  int32 setTextureCubemap(Texture& texture, Camera& camera);
  int32 setExposure(float exposure);
  int32 setGammaCorrection(float gamma);

  /////////////NEED TO PASS BRDF, IRRADIANCE AND PREFILTERED TEXTURES TO IBL MATERIAL
  //int32 setSamplerIrradiance()

  void updateMaterialSettings(glm::mat4 model, const uint32 buffer_offset, const uint64_t buffer_padding);

  int32 setRoughness(float);
  int32 setMetallic(float);

  UniformBlocks& getMaterialSettings();

protected:
  virtual ~Material();

private:
  Material(const Material&);
  int32 materialId_;
  MaterialType type_;
  UniformBlocks* settings_;

  friend class ResourceManager;
};

#endif // __MATERIAL__
