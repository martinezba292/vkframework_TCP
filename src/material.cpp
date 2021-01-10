#include "material.h"
#include <stdexcept>
#include "Components/texture.h"
#include "internal.h"
#include "resource_manager.h"

Material::Material()
{
  materialId_ = -1;
  type_ = kMaterialType_NONE;
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
}

int32 Material::getMaterialId()
{
  return materialId_;
}

int32 Material::getMaterialType()
{
  return type_;
}

void Material::setMaterialType(MaterialType type)
{
  if (type_ >= 0) 
    throw std::runtime_error("Already initialized");

  type_ = type;
}

int32 Material::setMaterialColor(glm::vec3 color)
{
  if (type_ >= kMaterialType_TextureSampler)
    throw std::runtime_error("Wrong material type");

  settings_->unlitBlock.color = glm::vec4(color, 1.0f);
  return 0;
}

int32 Material::setMaterialTexture(Texture texture)
{
  if (texture.getId() < 0)
    throw std::runtime_error("Texture hasn't been created yet");

  if (type_ < kMaterialType_TextureSampler)
    throw std::runtime_error("Wrong material type");

  InternalMaterial* mat = &ResourceManager::Get()->getResources()->internalMaterials[type_];
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

int32 Material::setRoughness(float rough)
{
  if (type_ != kMaterialType_BasicPBR) {
    printf("Wrong material type");
    return -1;
  }

  settings_->pbrBlock.roughness = rough;
  return 0;
}

int32 Material::setMetallic(float metal)
{
  if (type_ != kMaterialType_BasicPBR) {
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
