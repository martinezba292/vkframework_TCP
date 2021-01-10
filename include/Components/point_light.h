#ifndef __POINT_LIGHT__
#define __POINT_LIGHT__ 1


#include "Components/component.h"
#include "glm/glm.hpp"


class PointLight : public Component {
public:
  PointLight();
  ~PointLight();
  PointLight(const PointLight&) {}

private:


};

#endif
