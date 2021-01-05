#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__ 1

class VertexBuffer;
class Entity;
struct Resources;
class Material;
class Texture;
class ResourceManager {
public:
  ResourceManager();
  ~ResourceManager();
  static ResourceManager* Get();
  Resources* getResources() const;
  void initPrimitiveGeometries();

  void createVertexBuffer(VertexBuffer*);
  void createEntity(Entity*);
  void createMaterial(Material*);
  void createTexture(Texture*);

private:
  Resources* resources_;
  static ResourceManager* instance_;
};

#endif