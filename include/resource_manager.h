#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__ 1

class VertexBuffer;
class Entity;
struct Resources;
class Material;
class Texture;
class Camera;
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
  //void createCubemap(const char* texture_path, TextureFormat format);
  //Texture* getCubemapTexture();


private:
  ResourceManager();
  Resources* resources_;
  static ResourceManager* instance_;
};

#endif