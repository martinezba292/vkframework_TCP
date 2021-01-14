#include "Components/point_light.h"
#include "Components/transform.h"
#include "dev/internal.h"

PointLight::PointLight()
{
  type_ = ComponentType::kComponentType_Light;
  lightTransform_.alloc();
  params_.lightColor = glm::vec4(1.0f);
}

PointLight::PointLight(const PointLight& other)
{
  params_ = other.params_;
  PtrAlloc<Transform> tr = other.lightTransform_;
  lightTransform_ = tr;
}

void PointLight::setPosition(glm::vec3 position)
{
  lightTransform_->setPosition(position.x, position.y, position.z);
}

glm::vec3 PointLight::getPosition()
{
  return lightTransform_->getPosition();
}

void PointLight::setLightColor(glm::vec3 color)
{
  params_.lightColor = { color, 1.0f };
}

void PointLight::update(ComponentUpdateData* buffer)
{
  LightParams* light_block = &buffer->sceneBuffer.lights[buffer->sceneBuffer.lightNumber];
  light_block->lightPosition = { lightTransform_->getPosition() , 0.0f};
  light_block->lilghtColor = params_.lightColor;
  ++buffer->sceneBuffer.lightNumber;
  buffer->objectBuffer.unlitBlock.model = lightTransform_->getModel();
}


