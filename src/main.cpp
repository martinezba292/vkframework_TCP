//#define GLFW_INCLUDE_VULKAN
#include "vulkan_app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "stdio.h"

int main() {
  VulkanApp vulkan_app;
  vulkan_app.start();
  vulkan_app.loop();
  vulkan_app.end();

  return 0;
}