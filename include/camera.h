#ifndef __CAMERA_H__
#define __CAMERA_H__ 1

#include "glm/glm.hpp"

class Camera {
public:
  Camera();
  ~Camera(){}
  Camera(const Camera&);
  
  void setAspect(float width, float height);
  glm::vec3 getPosition();
  void cameraInput(float delta_time);
  void updateCamera();
  void setCameraSpeed(float speed);
  glm::mat4 getView();
  glm::mat4 getProjection();

private:
  glm::vec3 position_;
  glm::vec3 forward_;
  glm::vec3 right_;
  glm::vec3 up_;
  float pitch_;
  float yaw_;
  float lastCoords_[2];
  float speed_;
  float aspect_;
};

#endif // __CAMERA_H__ 1
