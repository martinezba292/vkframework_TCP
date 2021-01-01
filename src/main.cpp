#include "vulkan_app.h"


int main() {
  VulkanApp vulkan_app;
  vulkan_app.start();
  vulkan_app.loop();
  vulkan_app.end();

  return 0;
}