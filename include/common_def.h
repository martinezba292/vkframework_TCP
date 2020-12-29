#ifndef __COMMON_DEF__
#define __COMMON_DEF__ 1

#include <stdint.h>
#include <vector>
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#define NDEBUG

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef GLFWwindow Window;

static const int32 k_wWidth = 800;
static const int32 k_wHeight = 600;

static const int32 k_max_frames = 2;

static std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> deviceExtensions = {
  "VK_KHR_swapchain"
};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif


#endif
