#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__ 1

#include "Components/component.h"
#include "glm/glm.hpp"

struct ModelParams {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

struct ComponentUpdateData;
class Transform : public Component {
public:
  Transform();

  void setPosition(float x, float y, float z);
  void rotateX(float angle);
  void rotateY(float angle);
  void rotateZ(float angle);
  void setScale(float sx, float sy, float sz);
  void update(ComponentUpdateData*) override;

  glm::vec3 getPosition();
  glm::vec3 getRotation();
  glm::vec3 getScale();

  glm::mat4 getModel();

protected:
  virtual ~Transform(){}

private:
  Transform(const Transform&);
  ModelParams transform_data_;
};

#endif // __TRANSFORM_H__
