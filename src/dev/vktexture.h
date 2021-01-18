#ifndef __VKDEV_TEXTURE__
#define __VKDEV_TEXTURE__ 1

#include "vulkan/vulkan.h"
#include "common_def.h"

struct Context;
namespace vkdev {
  class VkTexture {
  public:
    VkTexture();
    ~VkTexture();
    VkTexture(const VkTexture&){}

    void loadCubemapKtx(Context* context, const char* filepath, VkFormat format);
    void loadImage(Context* context, const char* texture_path);
    void createImage(VkPhysicalDevice pdevice, VkFormat format, VkImageUsageFlags usage, uint32 layers, VkImageCreateFlags flags);
    //void createImageView(VkFormat format, uint32 layers, VkImageViewType view_type, VkImageAspectFlags flags);

    VkDevice device_;
    VkImage image_;
    VkImageLayout layout_;
    VkImageView view_;
    VkDeviceMemory memory_;
    uint32 width_, height_;
    uint32 mipLevels_;
    uint32 layerCount_;
    VkDescriptorImageInfo descriptor_;
    VkSampler sampler_;

  private:
    //void createSampler(Context* context, VkSamplerAddressMode address_mode, VkCompareOp compare_op, VkBorderColor border);
    void setImageLayout(Context* context, 
                        VkImageLayout old_layout, 
                        VkImageLayout new_layout, 
                        VkImageSubresourceRange subresource_range, 
                        VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
                        VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  };
}
#endif
