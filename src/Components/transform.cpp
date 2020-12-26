#include "Components/transform.h"


Transform::Transform()
{
  type_ = ComponentType::kComponentType_Transform;
  transform_data_.position = { 0.0f, 0.0f, 0.0f };
  transform_data_.rotation = { 0.0f, 0.0f, 0.0f };
  transform_data_.scale = { 1.0f, 1.0f, 1.0f };
}

Transform::Transform(const Transform& other)
{
  transform_data_ = other.transform_data_;
  type_ = other.type_;
  id_ = other.id_;
}

void Transform::setPosition(float x, float y, float z)
{
  transform_data_.position = { x, y, z };
}

void Transform::rotateX(float angle)
{
  transform_data_.rotation.x = angle;
}

void Transform::rotateY(float angle)
{
  transform_data_.rotation.y = angle;
}

void Transform::rotateZ(float angle)
{
  transform_data_.rotation.z = angle;
}

void Transform::setScale(float sx, float sy, float sz)
{
  transform_data_.scale = { sx, sy, sz };
}

glm::vec3 Transform::getPosition()
{
  return transform_data_.position;
}

glm::vec3 Transform::getRotation()
{
  return transform_data_.rotation;
}

glm::vec3 Transform::getScale()
{
  return transform_data_.scale;
}

glm::mat4 Transform::getModel()
{
  glm::vec3 pos = transform_data_.position;
  glm::mat4 translation{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    pos.x, pos.y, pos.z, 1.0f
  };

  glm::vec3 s = transform_data_.scale;
  glm::mat4 scale{
    s.x,  0.0f, 0.0f, 0.0f,
    0.0f, s.y,  0.0f, 0.0f,
    0.0f, 0.0f, s.z,  0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  glm::vec3 rot = transform_data_.rotation;
  glm::mat4 zRotation{
    cos(rot.z),  -sin(rot.z), 0.0f, 0.0f,
    sin(rot.z), cos(rot.z), 0.0f, 0.0f,
    0.0f,        0.0f,       1.0f, 0.0f,
    0.0f,        0.0f,       0.0f, 1.0f
  };

  glm::mat4 yRotation{
    cos(rot.y),  0.0f, sin(rot.y), 0.0f,
    0.0f,        1.0f, 0.0f,       0.0f,
    -sin(rot.y), 0.0f, cos(rot.y), 0.0f,
    0.0f,        0.0f, 0.0f,       1.0f
  };

  glm::mat4 xRotation{
    1.0f, 0.0f,       0.0f,        0.0f,
    0.0f, cos(rot.x), -sin(rot.x), 0.0f,
    0.0f, sin(rot.x), cos(rot.x),  0.0f,
    0.0f, 0.0f,       0.0f,        1.0f
  };


  return translation * (zRotation * yRotation * xRotation) * scale;
}
