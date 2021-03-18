#ifndef __POINT_LIGHT__
#define __POINT_LIGHT__ 1


#include "Components/component.h"
#include "glm/glm.hpp"
#include "dev/ptr_alloc.h"

struct PointLightData {
  glm::vec4 lightColor;
};

class Transform;
//struct UpdateData;
class PointLight : public Component {
public:
  PointLight();

  void setPosition(glm::vec3 position);
  glm::vec3 getPosition();
  void setLightColor(glm::vec3 color);
  void update(UpdateData*) override;

protected:
  virtual ~PointLight(){}

private:
  PointLight(const PointLight&);
  PtrAlloc<Transform> lightTransform_;
  PointLightData params_;
};

#endif
