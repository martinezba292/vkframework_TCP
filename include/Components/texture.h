#ifndef __TEXTURE_H__
#define __TEXTURE_H__ 1

#include "common_def.h"
#include <string>
#include "dev/referenced.h"

enum class TextureType {
  kTextureType_NONE = -1,
  kTextureType_2D = 0,
  kTextureType_Cubemap,
  kTextureType_MAX,
};

enum class TextureFormat {
  kTextureFormat_RGBA8_SRGB,
  kTextureFormat_RGBA16_FLOAT,
  kTextureFormat_RGBA32_FLOAT,
  kTextureFormat_RGB16_FLOAT,
  kTextureFormat_RGB32_FLOAT,
  kTextureFormat_RGBA8_UNORM,
  kTextureFormat_RGBA16_UNORM,
};

class Texture : public Referenced {
public:
  Texture();
  void loadTexture(const char* path, TextureType type, TextureFormat format);
  int16 getId();
  std::string getPath();
  TextureFormat getFormat();
  TextureType getType();

protected:
  virtual ~Texture(){}

private:
  Texture(const Texture&);
  int32 id_;
  TextureType type_;
  TextureFormat format_;
  std::string path_;

  friend class ResourceManager;
};

#endif // __TEXTURE_H__
