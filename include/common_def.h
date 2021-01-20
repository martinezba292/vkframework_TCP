#ifndef __COMMON_DEF__
#define __COMMON_DEF__ 1

#include <stdint.h>
#include <vector>
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#define VALIDATION_LAYERS

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef GLFWwindow Window;

static const int32 k_wWidth = 1024;
static const int32 k_wHeight = 768;

static const int32 k_max_frames = 2;

static std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> deviceExtensions = {
  "VK_KHR_swapchain"
};

#ifdef VALIDATION_LAYERS
  const bool enableValidationLayers = true;
#else
  const bool enableValidationLayers = false;
#endif


#endif
