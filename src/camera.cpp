#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "input_manager.h"
#define GLM_FORCE_RADIANS

Camera::Camera()
{
  position_ = { 0.0f, 0.0f, 20.0f };
  forward_ = { 0.0f, 0.0f, -1.0f };
  right_ = { 1.0f, 0.0f, 0.0f };
  up_ = { 0.0f, 1.0f, 0.0f };
  input_ = glm::vec3(0.0f);
  speed_ = 5.0f;
  aspect_ = k_wWidth / (float)k_wHeight;
  lastCoords_ = { 400.0f, 300.0f };
  yaw_ = -75.3f;
  pitch_ = 14.5f;
}

Camera::Camera(const Camera& other)
{
  position_ = other.position_;
  forward_ = other.forward_;
  right_ = other.right_;
  up_ = other.up_;
  input_ = other.input_;
  speed_ = other.speed_;
  aspect_ = other.aspect_;
  lastCoords_ = other.lastCoords_;
  yaw_ = other.yaw_;
  pitch_ = other.pitch_;
}

void Camera::setAspect(float width, float height)
{
  aspect_ = width / (float)height;
}

glm::vec3 Camera::getPosition()
{
  return position_;
}

void Camera::cameraInput(float delta_time)
{
  float* mousePos = InputManager::getCursorPosition();
  float sensitivity = 0.1f;
  float xoffset = (mousePos[0] - lastCoords_.x) * sensitivity;
  float yoffset = (mousePos[1] - lastCoords_.y) * sensitivity;
  lastCoords_ = { mousePos[0], mousePos[1] };

  yaw_ += xoffset;
  pitch_ += yoffset;

  if (pitch_ > 89.0f) {
    pitch_ = 89.0f;
  }
  if (pitch_ < -89.0f) {
    pitch_ = -89.0f;
  }

  float velocity = speed_ * delta_time;
  if (InputManager::getInputState(kKeyCode_W)) {
    input_ += forward_ * velocity;
  } else if (InputManager::getInputState(kKeyCode_S)) {
    input_ -= forward_ * velocity;
  }
  if (InputManager::getInputState(kKeyCode_A)) {
    input_ -= right_ * velocity;
  } else if (InputManager::getInputState(kKeyCode_D)) {
    input_ += right_ * velocity;
  }
}

void Camera::updateCamera()
{
  position_ += input_;
  input_ = glm::vec3(0.0f);

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  direction.y = sin(glm::radians(pitch_));
  direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  forward_ = glm::normalize(direction);

  glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
  right_ = glm::normalize(glm::cross(forward_, worldUp));
  up_ = glm::normalize(glm::cross(right_, forward_));
}

void Camera::setCameraSpeed(float speed)
{
  speed_ = speed;
}

glm::mat4 Camera::getView()
{
  return glm::lookAt(position_, position_ + forward_, up_);
}

glm::mat4 Camera::getProjection()
{
  return glm::perspective(glm::radians(45.0f), aspect_, 0.1f, 100.0f);
}

