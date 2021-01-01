#include "material.h"

Material::Material()
{
  materialId_ = -1;
  type_ = kMaterialType_NONE;
}

Material::Material(const Material& other)
{
  materialId_ = other.materialId_;
  type_ = other.type_;
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
  type_ = type;
}
