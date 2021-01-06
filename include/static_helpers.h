#ifndef __STATIC_HELPERS__
#define __STATIC_HELPERS__ 1

#include "vulkan/vulkan.h"
#include "common_def.h"
#include <fstream>

struct QueueFamilyIndices;
struct SwapChainSupportDetails;
struct Context;
struct InternalMaterial;
struct InternalTexture;
class Texture;
enum LayoutType;
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

  static VkPipeline createPipeline(Context* context, const char* vert_path, const char* frag_path, VkPipelineLayout pipelineLayout);


  static InternalTexture createTextureImage(Context* context, Texture texture);

  static VkImageView createTextureImageView(Context* context, VkImage image, VkFormat format, VkImageAspectFlags flags);

  static VkSampler createTextureSampler(Context* context);

  static void createImage(Context* context, uint32 width, uint32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                          VkMemoryPropertyFlags property, VkImage& image, VkDeviceMemory& image_memory);

  static VkCommandBuffer beginSingleTimeCommands(Context* context);

  static void endSingleTimeCommands(Context* context, VkCommandBuffer command_buffer);

  static void transitionImageLayout(Context* context, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

  static void copyBufferToImage(Context* context, VkBuffer buffer, VkImage image, uint32 width, uint32 height);

  static void destroyMaterial(Context* context, InternalMaterial* material);

  static VkFormat findSupportedFormats(Context* context, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  static VkFormat findDepthFormat(Context* context);
};


#endif // __STATIC_HELPERS__
