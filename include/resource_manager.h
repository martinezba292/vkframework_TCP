#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__ 1

class VertexBuffer;
struct Resources;
class ResourceManager {
public:
  ResourceManager();
  ~ResourceManager();
  static ResourceManager* Get();
  Resources* getResources() const;

  void createVertexBuffer(VertexBuffer*);

private:
  Resources* resources_;
  static ResourceManager* instance_;
};

#endif