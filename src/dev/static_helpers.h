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
enum class TextureFormat;

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

    VkPipeline createPipeline(Context* context, 
                              const char* vert_path, 
                              const char* frag_path, 
                              VkPipelineLayout pipeline_layout, 
                              VkCullModeFlags cull_mode, 
                              VkBool32 depth_test, 
                              uint8 vertex_desc = 3);


    VkFormat getTextureFormat(TextureFormat format);

    VkImageView createTextureImageView(VkDevice device, 
                                       VkImage& image, 
                                       VkFormat format, 
                                       VkImageViewType view_type, 
                                       uint32 mip_levels, 
                                       uint32 layers, 
                                       VkImageAspectFlags flags,
                                       VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_R, 
                                                                      VK_COMPONENT_SWIZZLE_G, 
                                                                      VK_COMPONENT_SWIZZLE_B, 
                                                                      VK_COMPONENT_SWIZZLE_A });

    VkSampler createTextureSampler(Context* context, VkSamplerAddressMode address_mode, VkCompareOp compare_op, uint32 mip_levels, VkBorderColor border, VkBool32 anisotropy = VK_TRUE);


    VkCommandBuffer beginSingleTimeCommands(Context* context);

    void endSingleTimeCommands(Context* context, VkCommandBuffer command_buffer);

    VkWriteDescriptorSet descriptorWriteInitializer(uint32 binding, 
                                                    VkDescriptorType type, 
                                                    VkDescriptorSet& descriptor_set, 
                                                    VkDescriptorBufferInfo* buffer_info,
                                                    uint32 descriptor_count = 1);

    VkWriteDescriptorSet descriptorWriteInitializer(uint32 binding, 
                                                    VkDescriptorType type, 
                                                    VkDescriptorSet& descriptor_set, 
                                                    VkDescriptorImageInfo* image_info,
                                                    uint32 descriptor_count = 1);

    VkDescriptorSetLayoutBinding layoutBindingInitializer(VkDescriptorType type, VkShaderStageFlags flags, 
                                                          uint32 binding, uint32 descriptor_count = 1);

    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfoInitializer(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

    void destroyMaterial(Context* context, InternalMaterial* material);
  }
}

#endif // __STATIC_HELPERS__
