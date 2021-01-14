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

namespace dev {
  namespace StaticHelpers {
    uint32 findMemoryType(VkPhysicalDevice device, uint32 typeFilter, VkMemoryPropertyFlags properties);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR& surface);

    VkFormat findSupportedFormats(Context* context, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat(Context* context);

    uint64_t padUniformBufferOffset(Context* context, size_t size);


    std::vector<char> loadShader(const std::string& filename);

    VkShaderModule createShaderModule(const VkDevice device, const std::vector<char>& code);

    SwapChainSupportDetails querySwapChain(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkPipeline createPipeline(Context* context, const char* vert_path, const char* frag_path, VkPipelineLayout pipelineLayout);


    InternalTexture createTextureImage(Context* context, const char* texture_path);

    VkImageView createTextureImageView(Context* context, VkImage image, VkFormat format, VkImageAspectFlags flags);

    VkSampler createTextureSampler(Context* context);

    void createImage(Context* context, uint32 width, uint32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
      VkMemoryPropertyFlags property, VkImage& image, VkDeviceMemory& image_memory);

    VkCommandBuffer beginSingleTimeCommands(Context* context);

    void endSingleTimeCommands(Context* context, VkCommandBuffer command_buffer);

    void transitionImageLayout(Context* context, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    void copyBufferToImage(Context* context, VkBuffer buffer, VkImage image, uint32 width, uint32 height);

    void destroyMaterial(Context* context, InternalMaterial* material);
  }
}

#endif // __STATIC_HELPERS__
