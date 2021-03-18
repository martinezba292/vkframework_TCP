#include "Components/texture.h"

Texture::Texture()
{
  id_ = -1;
}

Texture::Texture(const Texture& other)
{
  id_ = other.id_;
  path_ = other.path_;
}

void Texture::loadTexture(const char* path, TextureType type, TextureFormat format)
{
  format_ = format;
  type_ = type;
  path_ = path;
}

int16 Texture::getId()
{
  return id_;
}

std::string Texture::getPath()
{
  return path_;
}

TextureFormat Texture::getFormat()
{
  return format_;
}

TextureType Texture::getType()
{
  return type_;
}

