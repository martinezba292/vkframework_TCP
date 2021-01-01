#ifndef __CAMERA_H__
#define __CAMERA_H__ 1

#include "glm/glm.hpp"

class Camera {
public:
  Camera();
  ~Camera(){}
  Camera(const Camera&);
  
  void setAspect(float width, float height);
  void setCameraSpeed(float speed);
  glm::vec3 getPosition();
  glm::mat4 getView();
  glm::mat4 getProjection();
  void cameraInput(float delta_time);
  void updateCamera();

private:
  glm::vec3 position_;
  glm::vec3 forward_;
  glm::vec3 right_;
  glm::vec3 up_;
  glm::vec3 input_;
  glm::vec2 lastCoords_;
  float pitch_;
  float yaw_;
  float speed_;
  float aspect_;
};

#endif // __CAMERA_H__ 1
