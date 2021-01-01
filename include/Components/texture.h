#ifndef __TEXTURE_H__
#define __TEXTURE_H__ 1

#include "common_def.h"
#include <string>

class Texture {
public:
  Texture();
  ~Texture(){}
  Texture(const Texture&);

  void loadTexture(const char* path);
  int16 getId();
  std::string getPath();
private:
  int16 id_;
  std::string path_;

};

#endif // __TEXTURE_H__
