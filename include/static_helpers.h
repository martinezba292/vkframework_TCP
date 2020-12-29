#ifndef __STATIC_HELPERS__
#define __STATIC_HELPERS__ 1

#include "vulkan/vulkan.h"
#include "common_def.h"
#include <fstream>

struct QueueFamilyIndices;
struct SwapChainSupportDetails;
struct Context;

class StaticHelpers {
public:
  static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface);

  static uint32 findMemoryType(VkPhysicalDevice device, uint32 typeFilter, VkMemoryPropertyFlags properties);

  static void createInternalBuffer(Context context, VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

  static void copyBuffer(Context context, VkBuffer srcBuffer, VkBuffer dstBuffer, 
                         VkDeviceSize bufferSize, VkDeviceSize dstOffset = 0);

  static uint64_t padUniformBufferOffset(Context* context, size_t size);

  static std::vector<char> loadShader(const std::string& filename);

  static VkShaderModule createShaderModule(const VkDevice device, const std::vector<char>& code);

  static SwapChainSupportDetails querySwapChain(VkPhysicalDevice device, VkSurfaceKHR surface);
};


#endif // __STATIC_HELPERS__
