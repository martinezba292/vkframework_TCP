#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__ 1

class VertexBuffer;
class Entity;
struct Resources;
class ResourceManager {
public:
  ResourceManager();
  ~ResourceManager();
  static ResourceManager* Get();
  Resources* getResources() const;
  void initPrimitiveGeometries();

  void createVertexBuffer(VertexBuffer*);
  void createEntity(Entity*);

private:
  Resources* resources_;
  static ResourceManager* instance_;
};

#endif