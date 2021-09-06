#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__ 1

#include <string>
#include <list>
#include "dev/ptr_alloc.h"
#include "common_def.h"

class VertexBuffer;
class Entity;
struct Resources;
class Material;
class Texture;
class Camera;
class Geometry;
//enum class TextureFormat;
class ResourceManager {
public:
  ~ResourceManager();
  static ResourceManager* Get();
  Resources* getResources() const;
  void initPrimitiveGeometries();

  void createVertexBuffer(VertexBuffer*);
  void createEntity(Entity*);
  void createMaterial(Material*);
  void createTexture(Texture*);
  Camera& getCamera();
  std::list<PtrAlloc<Geometry>> loadObj(std::string path);


private:
  ResourceManager();
  Resources* resources_;
  static ResourceManager* instance_;
  void terrainGenerator(uint32 w, uint32 h);
  void sphereGenerator(float longitude_rev, float latitude_rev);

};

#endif