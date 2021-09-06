#include "material.h"
#include <stdexcept>
#include "Components/texture.h"
#include "dev/internal.h"
#include "resource_manager.h"
#include "camera.h"

Material::Material()
{
  materialId_ = -1;
  type_ = MaterialType::kMaterialType_NONE;
  settings_ = new UniformBlocks();
}

Material::Material(const Material& other)
{
  materialId_ = other.materialId_;
  type_ = other.type_;
  *settings_ = *other.settings_;
}

Material::~Material()
{
  delete(settings_);
}

int32 Material::getMaterialId()
{
  return materialId_;
}

int32 Material::getMaterialType()
{
  return (int32)type_;
}

void Material::setMaterialType(MaterialType type)
{
  if ((int32)type_ >= 0)
    throw std::runtime_error("Already initialized");

  type_ = type;
}

int32 Material::setMaterialColor(glm::vec3 color)
{
  if (type_ >= MaterialType::kMaterialType_TextureSampler)
    throw std::runtime_error("Wrong material type");

  settings_->unlitBlock.albedo = glm::vec4(color, 1.0f);
  return 0;
}

int32 Material::setMaterialTexture(Texture& texture)
{
  if (texture.getId() < 0) {
    printf("Texture hasn't been created yet");
    return -1;
  }

  if (type_ < MaterialType::kMaterialType_TextureSampler) {
    printf("Wrong material type");
    return -1;
  }

  if (texture.getType() != TextureType::kTextureType_2D) {
    printf("Wrong texture type");
    return -1;
  }

  InternalMaterial* mat = &ResourceManager::Get()->getResources()->internalMaterials[(int32)type_];
  auto result = std::find(mat->texturesReferenced.begin(), mat->texturesReferenced.end(), texture.getId());
  if (result == mat->texturesReferenced.end()) {
    settings_->textureBlock.textureIndex = mat->texturesReferenced.size();
    mat->texturesReferenced.push_back(texture.getId());
    return 0;
  }
  uint32 index = result - mat->texturesReferenced.begin();
  settings_->textureBlock.textureIndex = index;

  return 0;
}

int32 Material::setTextureCubemap(Texture& texture, Camera& camera)
{
  if (type_ != MaterialType::kMaterialType_Skybox) {
    printf("Wrong Material type");
    return -1;
  }

  if (texture.getType() != TextureType::kTextureType_Cubemap) {
    printf("Wrong texture type");
    return -1;
  }
  InternalMaterial* mat = &ResourceManager::Get()->getResources()->internalMaterials[(int32)type_];
  auto result = std::find(mat->texturesReferenced.begin(), mat->texturesReferenced.end(), texture.getId());
  if (result != mat->texturesReferenced.end()) {
    printf("Texture already in use");
    return -1;
  }

  mat->texturesReferenced.push_back(texture.getId());
  settings_->skyboxBlock.viewStatic = camera.getView();
  settings_->skyboxBlock.viewStatic[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  return 0;
}


int32 Material::setExposure(float exposure)
{
  if (type_ != MaterialType::kMaterialType_PBRIBL) {
    throw std::runtime_error("Wrong material type");
    return -1;
  }

  settings_->pbriblBlock.exposure = exposure;
  return 0;
}

int32 Material::setGammaCorrection(float gamma)
{
  if (type_ != MaterialType::kMaterialType_PBRIBL) {
    throw std::runtime_error("Wrong material type");
    return -1;
  }

  settings_->pbriblBlock.gamma = gamma;
  return 0;
}

int32 Material::setRandomNoise(float rand)
{
  if (type_ != MaterialType::kMaterialType_Noise) {
    printf("Wrong Material type");
    return -1;
  }

  settings_->noiseBlock.randc = rand;
  return 0;
}

int32 Material::setNoiseAmplification(float amp)
{
  if (type_ != MaterialType::kMaterialType_Noise) {
    printf("Wrong Material type");
    return -1;
  }

  settings_->noiseBlock.amplification = amp;
  return 0;
}

void Material::updateMaterialSettings(glm::mat4 model, const uint32 buffer_offset, const uint64_t buffer_padding)
{
  ResourceManager* rm = ResourceManager::Get();
  switch (type_) {
  case MaterialType::kMaterialType_Skybox: {
    settings_->skyboxBlock.viewStatic = rm->getCamera().getView();
    settings_->skyboxBlock.viewStatic[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    break;
  }
  default: {
    settings_->unlitBlock.model = model;
    break;
  }
  }

  InternalMaterial* mat = &rm->getResources()->internalMaterials[(int32)type_];
  UniformBlocks* uniform_buffer = (UniformBlocks*)((uint64_t)mat->dynamicUniformData +
                                                   (buffer_offset * buffer_padding));
  *uniform_buffer = *settings_;
}

int32 Material::setRoughness(float rough)
{
  if (type_ < MaterialType::kMaterialType_BasicPBR ||
      type_ > MaterialType::kMaterialType_PBRIBL) {
    printf("Wrong material type");
    return -1;
  }

  settings_->pbrBlock.roughness = rough;
  return 0;
}

int32 Material::setMetallic(float metal)
{
  if (type_ < MaterialType::kMaterialType_BasicPBR || 
      type_ > MaterialType::kMaterialType_PBRIBL) {
    printf("Wrong material type");
    return -1;
  }

  settings_->pbrBlock.metallic = metal;
  return 0;
}


UniformBlocks& Material::getMaterialSettings()
{
  return *settings_;
}
