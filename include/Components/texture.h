#ifndef __TEXTURE_H__
#define __TEXTURE_H__ 1

#include "common_def.h"
#include <string>
#include "dev/referenced.h"

class Texture : public Referenced {
public:
  Texture();
  void loadTexture(const char* path);
  int16 getId();
  std::string getPath();

protected:
  virtual ~Texture(){}

private:
  Texture(const Texture&);
  int32 id_;
  std::string path_;

  friend class ResourceManager;
};

#endif // __TEXTURE_H__
