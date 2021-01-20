#include <stdexcept>
#include "dev/vktexture.h"
#include "ktxvulkan.h"
#include <fstream>
#include "dev/buffer.h"
#include "internal.h"
#include "static_helpers.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


vkdev::VkTexture::VkTexture()
{
  width_ = 0;
  height_ = 0;
  mipLevels_ = 1;
  layerCount_ = 1;
  layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
  device_ = VK_NULL_HANDLE;
}

vkdev::VkTexture::~VkTexture()
{
  destroyTexture();
}

void vkdev::VkTexture::loadCubemapKtx(Context* context, const char* filepath, VkFormat format)
{
  std::ifstream f(filepath);
  if (f.fail())
    throw std::runtime_error("Failed to open the texture cubemap");

  ktxResult result;
  ktxTexture* ktx_texture;
  uint32 layer_count = 6;
  device_ = context->logDevice_;

  result = ktxTexture_CreateFromNamedFile(filepath, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
  width_ = ktx_texture->baseWidth;
  height_ = ktx_texture->baseHeight;
  mipLevels_ = ktx_texture->numLevels;
  ktx_uint8_t* ktx_texture_data = ktxTexture_GetData(ktx_texture);
  ktx_size_t ktx_texture_size = ktxTexture_GetSize(ktx_texture);

  vkdev::Buffer staging_buffer;
  staging_buffer.createBuffer(context, ktx_texture_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* data;
  vkMapMemory(device_, staging_buffer.memory_, 0, ktx_texture_size, 0, &data);
  memcpy(data, ktx_texture_data, ktx_texture_size);
  vkUnmapMemory(device_, staging_buffer.memory_);


  createImage(context->physDevice_, format, 
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              layer_count, 
              VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context);

  std::vector<VkBufferImageCopy> copyRegions;
  uint32 offset = 0;

  for (size_t face = 0; face < layer_count; face++) {
    for (size_t levels = 0; levels < mipLevels_; levels++) {
      ktx_size_t offset;
      KTX_error_code ret = ktxTexture_GetImageOffset(ktx_texture, levels, 0, face, &offset);
      //assert(ret == KTX_SUCCESS);
      VkBufferImageCopy region{};
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = levels;
      region.imageSubresource.baseArrayLayer = face;
      region.imageSubresource.layerCount = 1;
      region.imageExtent.width = width_ >> levels;
      region.imageExtent.height = height_ >> levels;
      region.imageExtent.depth = 1;
      region.bufferOffset = offset;
      copyRegions.push_back(region);
    }
  }

  VkImageSubresourceRange subresource_range{};
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = mipLevels_;
  subresource_range.layerCount = layer_count;

  setImageLayout(context, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

  vkCmdCopyBufferToImage(cmd_buffer, 
                         staging_buffer.buffer_, 
                         image_, 
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                         static_cast<uint32>(copyRegions.size()), 
                         copyRegions.data());
  dev::StaticHelpers::endSingleTimeCommands(context, cmd_buffer);



  sampler_ = dev::StaticHelpers::createTextureSampler(context, 
                                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 
                                                      VK_COMPARE_OP_NEVER, 
                                                      mipLevels_, 
                                                      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

  view_ = dev::StaticHelpers::createTextureImageView(device_, image_, format, 
                                                     VK_IMAGE_VIEW_TYPE_CUBE, 
                                                     mipLevels_, layer_count, 
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

  cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context);

  layout_ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  setImageLayout(context, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layout_, subresource_range);

  dev::StaticHelpers::endSingleTimeCommands(context, cmd_buffer);

  ktxTexture_Destroy(ktx_texture);
}

void vkdev::VkTexture::loadImage(Context* context, const char* texture_path)
{
  int32 texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(texture_path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  if (!pixels) {
    throw std::runtime_error("\nFailed to load image texture");
  }

  width_ = texWidth;
  height_ = texHeight;
  mipLevels_ = 1;
  device_ = context->logDevice_;
  VkDeviceSize imageSize = (uint64_t)(texWidth) * (uint64_t)(texHeight) * 4;
  vkdev::Buffer staging_buffer;
  staging_buffer.createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* data;
  vkMapMemory(device_, staging_buffer.memory_, 0, imageSize, 0, &data);
  memcpy(data, pixels, imageSize);
  vkUnmapMemory(device_, staging_buffer.memory_);

  stbi_image_free(pixels);

  createImage(context->physDevice_, 
              VK_FORMAT_R8G8B8A8_SRGB, 
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
              1, 0);

  VkImageSubresourceRange subresource_range{};
  subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource_range.baseMipLevel = 0;
  subresource_range.levelCount = 1;
  subresource_range.layerCount = 1;
  setImageLayout(context, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = { (uint32)texWidth, (uint32)texHeight, 1 };

  vkCmdCopyBufferToImage(cmd_buffer, staging_buffer.buffer_, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  dev::StaticHelpers::endSingleTimeCommands(context, cmd_buffer);
  
  setImageLayout(context, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource_range);
  
  view_ = dev::StaticHelpers::createTextureImageView(device_, 
                                                     image_, 
                                                     VK_FORMAT_R8G8B8A8_SRGB, 
                                                     VK_IMAGE_VIEW_TYPE_2D, 
                                                     1, 1, 
                                                     VK_IMAGE_ASPECT_COLOR_BIT);

  sampler_ = dev::StaticHelpers::createTextureSampler(context, 
                                                      VK_SAMPLER_ADDRESS_MODE_REPEAT, 
                                                      VK_COMPARE_OP_ALWAYS, 
                                                      1, 
                                                      VK_BORDER_COLOR_INT_OPAQUE_BLACK);
}

void vkdev::VkTexture::destroyTexture()
{
  if (device_) {
    vkDestroyImageView(device_, view_, nullptr);
    vkDestroyImage(device_, image_, nullptr);
    vkFreeMemory(device_, memory_, nullptr);
    vkDestroySampler(device_, sampler_, nullptr);
    device_ = VK_NULL_HANDLE;
  }
}

void vkdev::VkTexture::createImage(VkPhysicalDevice pdevice, VkFormat format, VkImageUsageFlags usage, uint32 layers, VkImageCreateFlags flags)
{
  VkImageCreateInfo image_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format;
  image_info.mipLevels = mipLevels_;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.extent = { width_, height_, 1 };
  image_info.usage = usage;
  image_info.arrayLayers = layers;
  image_info.flags = flags;

  //assert(vkCreateImage(device_, &image_info, nullptr, &image_) == VK_SUCCESS);
  VkResult res = vkCreateImage(device_, &image_info, nullptr, &image_);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_, image_, &memRequirements);

  VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = dev::StaticHelpers::findMemoryType(pdevice, memRequirements.memoryTypeBits,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  //assert(vkAllocateMemory(device_, &allocInfo, nullptr, &memory_) == VK_SUCCESS);
  res = vkAllocateMemory(device_, &allocInfo, nullptr, &memory_);
  vkBindImageMemory(device_, image_, memory_, 0);
}


void vkdev::VkTexture::setImageLayout(Context* context, 
                                      VkImageLayout old_layout, 
                                      VkImageLayout new_layout, 
                                      VkImageSubresourceRange subresource_range, 
                                      VkPipelineStageFlags src_stage_mask, 
                                      VkPipelineStageFlags dst_stage_mask)
{
  
  VkCommandBuffer cmd_buffer = dev::StaticHelpers::beginSingleTimeCommands(context);

  VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_;
  barrier.subresourceRange = subresource_range;

  switch (old_layout) {
    case VK_IMAGE_LAYOUT_PREINITIALIZED: {
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    }
    default: {
      barrier.srcAccessMask = 0;
      break;
    }
  }

  switch (new_layout) {
    case VK_IMAGE_LAYOUT_PREINITIALIZED: {
      barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
      barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    }
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
      if (barrier.srcAccessMask == 0) {
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    }
    default: {
      break;
    }
  }

  vkCmdPipelineBarrier(cmd_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  dev::StaticHelpers::endSingleTimeCommands(context, cmd_buffer);

}

